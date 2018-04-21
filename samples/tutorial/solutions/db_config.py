import os
exec(open(os.path.join("..", "db_config.py"), "r").read())

import db_config as root_db_config
user = root_db_config.user
pw = root_db_config.pw
dsn = root_db_config.dsn
