# -*- coding: utf-8 -*-
import cx_Oracle
import sample_env

"The test below verifies that the option to work around saving and reading of inconsistent encodings works"

def ConvertStringToBytes(cursor, name, defaultType, size, precision, scale):
    if defaultType == cx_Oracle.STRING:
        return cursor.var(str, arraysize=cursor.arraysize, bypassencoding = True)    

connection = cx_Oracle.connect(sample_env.get_main_connect_string())
cursor = connection.cursor()

cursor.outputtypehandler = ConvertStringToBytes

sql = 'create table EncodingExperiment (content varchar2(100), encoding varchar2(15))'

print('Creating experiment table')
try:
    cursor.execute(sql)
    print('Success, will attempt to add records')
except Exception as err:
    # table already exists
    print('%s\n%s'%(err, 'EncodingExperiment table exists... Will attempt to add records'))

# variable that we will test encodings against
unicode_string = 'I bought a cafetière on the Champs-Élysées'

# First test
windows_1252_encoded = unicode_string.encode('windows-1252')
# Second test
utf8_encoded = unicode_string.encode('utf-8')

sqlparameters = [(windows_1252_encoded, 'windows-1252'), (utf8_encoded, 'utf-8')]

sql = 'insert into EncodingExperiment (content, encoding) values (:content, :encoding)'

# cx_Oracle string variable in which we will store byte value and insert it as such
content_variable = cursor.var(cx_Oracle.STRING)

print('Adding records to the table: "EncodingExperiment"')
for sqlparameter in sqlparameters:
    content, encoding = sqlparameter
    # setting content_variable value to a byte value and instert it as such
    content_variable.setvalue(0, content)
    cursor.execute(sql, content=content_variable, encoding=encoding)
    
sql = 'select * from EncodingExperiment'

print('Fetching records from table EncodingExperiment')
result = cursor.execute(sql).fetchall()

for dataset in result:
    content, encoding = dataset[0], dataset[1].decode()
    decodedcontent = content.decode(encoding)
    print('Is "%s" == "%s" ?\nResult: %s, (decoded from: %s)'%(decodedcontent, unicode_string, decodedcontent == unicode_string, encoding))

print('Finished testing, will attempt to drop the table "EncodingExperiment"')
# drop table after finished testing
sql = 'drop table EncodingExperiment'
try:
    cursor.execute(sql)
    print('Successfully droped table "EncodingExperiment" from database.')
except Exception as err:
    print('Failed to drop table from the database, info: %s'%err)










