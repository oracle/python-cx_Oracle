-- Default database username
def user = "pythonhol"

-- Default database connection string
def connect_string = "localhost/orclpdb1"

-- Prompt for the password
accept pw char prompt 'Enter database password for &user: ' hide
