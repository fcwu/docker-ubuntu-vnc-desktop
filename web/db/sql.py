import json
import logging
from peewee import (SqliteDatabase, Model,
                    CharField
                    )
from os.path import exists as pexists
import datetime
import sha


DATABASE = 'lightop.sqlite'
DB_USER_VERSION = 1
database = SqliteDatabase(DATABASE, threadlocals=True)


class BaseModel(Model):
    class Meta:
        database = database

    def __str__(self):
        r = {}
        for k in self._data.keys():
            try:
                r[k] = str(getattr(self, k))
            except:
                r[k] = json.dumps(getattr(self, k))
        return str(r)

    def serialize(self):
        r = {}
        for k in self._data.keys():
            try:
                # value = getattr(self, k)
                # if isinstance(value, int):
                #    r[k] = va
                r[k] = getattr(self, k)
                if isinstance(r[k], datetime.datetime):
                    r[k] = int((r[k] - datetime.datetime(1970, 1, 1))
                               .total_seconds())
            except:
                r[k] = json.dumps(getattr(self, k))
        return r

    def marshal(self):
        return self


class KeyValue(BaseModel):
    key = CharField()
    value = CharField()

    class Meta:
        order_by = ('key',)


class User(BaseModel):
    user = CharField(default='')
    password = CharField(default='')


def create_tables():
    database.connect()
    set_user_verion()
    database.create_tables([KeyValue, User])
    User.create(user='admin', password=sha.new('admin').hexdigest())


def connect():
    database.connect()


def close():
    database.close()


def set_user_verion():
    version = 'PRAGMA user_version = ' + str(DB_USER_VERSION)
    database.execute_sql(version)


def get_user_version():
    version = 'PRAGMA user_version'
    cursor = database.execute_sql(version)
    v = cursor.fetchone()
    logging.info('Existing database user version: ' + str(v[0]))
    return v[0]


if not pexists(DATABASE):
    create_tables()
else:
    v = get_user_version()
    if v < DB_USER_VERSION:
        logging.warn('Existing database version is outdated')
    elif v > DB_USER_VERSION:
        logging.warn("DB version doesn't match")
