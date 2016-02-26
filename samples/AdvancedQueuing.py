#------------------------------------------------------------------------------
# AdvancedQueuing.py
#   This script demonstrates how to use advanced queuing using cx_Oracle. It
# creates a simple type and enqueues and dequeues a few objects.
#------------------------------------------------------------------------------

from __future__ import print_function

CONNECT_STRING = "user/pw@tns"
BOOK_TYPE_NAME = "UDT_BOOK"
QUEUE_NAME = "BOOKS"
QUEUE_TABLE_NAME = "BOOK_QUEUE"

import cx_Oracle
import decimal

# connect to database
connection = cx_Oracle.Connection(CONNECT_STRING)
cursor = connection.cursor()

# drop queue table, if present
cursor.execute("""
        select count(*)
        from user_tables
        where table_name = :name""", name = QUEUE_TABLE_NAME)
count, = cursor.fetchone()
if count > 0:
    print("Dropping queue table...")
    cursor.callproc("dbms_aqadm.drop_queue_table", (QUEUE_TABLE_NAME, True))

# drop type, if present
cursor.execute("""
        select count(*)
        from user_types
        where type_name = :name""", name = BOOK_TYPE_NAME)
count, = cursor.fetchone()
if count > 0:
    print("Dropping books type...")
    cursor.execute("drop type %s" % BOOK_TYPE_NAME)

# create type
print("Creating books type...")
cursor.execute("""
        create type %s as object (
            title varchar2(100),
            authors varchar2(100),
            price number(5,2)
        );""" % BOOK_TYPE_NAME)

# create queue table and quueue and start the queue
print("Creating queue table...")
cursor.callproc("dbms_aqadm.create_queue_table",
        (QUEUE_TABLE_NAME, BOOK_TYPE_NAME))
cursor.callproc("dbms_aqadm.create_queue", (QUEUE_NAME, QUEUE_TABLE_NAME))
cursor.callproc("dbms_aqadm.start_queue", (QUEUE_NAME,))

# enqueue a few messages
booksType = connection.gettype(BOOK_TYPE_NAME)
book1 = booksType.newobject()
book1.TITLE = "The Fellowship of the Ring"
book1.AUTHORS = "Tolkien, J.R.R."
book1.PRICE = decimal.Decimal("10.99")
book2 = booksType.newobject()
book2.TITLE = "Harry Potter and the Philosopher's Stone"
book2.AUTHORS = "Rowling, J.K."
book2.PRICE = decimal.Decimal("7.99")
for book in (book1, book2):
    print("Enqueuing book", book.TITLE)
    cursor.execute("""
            declare
                t_MessageId             raw(16);
                t_Options               dbms_aq.enqueue_options_t;
                t_MessageProperties     dbms_aq.message_properties_t;
            begin
                dbms_aq.enqueue(
                        queue_name => :queueName,
                        enqueue_options => t_Options,
                        message_properties => t_MessageProperties,
                        payload => :book,
                        msgid => t_MessageId);
            end;""",
            queueName = QUEUE_NAME,
            book = book)
connection.commit()

# dequeue the messages
var = cursor.var(cx_Oracle.OBJECT, typename = BOOK_TYPE_NAME)
while True:
    cursor.execute("""
            declare
                t_MessageId             raw(16);
                t_Options               dbms_aq.dequeue_options_t;
                t_MessageProperties     dbms_aq.message_properties_t;
                no_messages             exception;
                pragma exception_init(no_messages, -25228);
            begin
                t_Options.navigation := dbms_aq.FIRST_MESSAGE;
                t_Options.wait := dbms_aq.NO_WAIT;
                begin
                    dbms_aq.dequeue(
                            queue_name => :queueName,
                            dequeue_options => t_Options,
                            message_properties => t_MessageProperties,
                            payload => :book,
                            msgid => t_MessageId);
                exception
                when no_messages then
                    :book := null;
                end;
            end;""",
            queueName = QUEUE_NAME,
            book = var)
    book = var.getvalue()
    if book is None:
        break
    print("Dequeued book", book.TITLE)

