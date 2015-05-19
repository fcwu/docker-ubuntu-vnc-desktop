__all__ = ['auth']
__version__ = '0.1'


from flask_login import (LoginManager,
                         login_required,
                         login_user,
                         current_user,
                         login_fresh,
                         logout_user,
                         AnonymousUserMixin,
                         )
from functools import wraps
from db.sql import User as DbUser
import sha


def noauth(func):
    @wraps(func)
    def with_logging(*args, **kwargs):
        return func(*args, **kwargs)
    return with_logging


class Auth(object):
    def init_app(self, app, method):
        self._login_manager = LoginManager()
        self._login_manager.session_protection = "basic"
        self._login_manager.init_app(app)
        self._login_manager.user_loader(self._load_user)
        self._login_manager.anonymous_user = Anonymous
        self.login_required = login_required
        self.login_fresh = login_fresh
        self.current_user = current_user

    def login(self, **kargs):
        if 'username' in kargs and 'password' in kargs:
            u = User.get(kargs['username'], kargs['password'])
            if u is None:
                return None
            login_user(u, remember=kargs['remember'])
            return u
        return None

    def unauthorized_handler(self, func):
        self._login_manager.unauthorized_handler(func)

    @staticmethod
    def _load_user(userid):
        u = User.get(userid, None)
        return u

    def logout(self, username):
        User.delete(username)
        return logout_user()


class User(object):
    _users = {}

    def __init__(self, userid):
        self._userid = userid
        self._is_admin = False

    @classmethod
    def get(cls, u, password):
        if password is None:
            if u in cls._users:
                return cls._users[u]
            return None
        if not cls.authenticate(u, password):
            return None
        user = User(u)
        if u == 'admin':
            user._is_admin = True
        cls._users[u] = user
        return user

    @classmethod
    def authenticate(cls, u, password):
        users = DbUser.select().where(DbUser.user == u)
        if users.count() <= 0:
            return False
        if users[0].password == sha.new(password).hexdigest():
            return True
        return False

    @classmethod
    def delete(cls, u):
        if u in cls._users:
            del cls._users[u]

    def is_authenticated(self):
        return True

    def is_active(self):
        return True

    def is_anonymous(self):
        return False

    def is_admin(self):
        return self._is_admin

    def get_id(self):
        return self._userid

    def username(self):
        return self._userid


class Anonymous(AnonymousUserMixin):
    def __init__(self):
        self._userid = 'Anonymous'

    def username(self):
        return self._userid

auth = Auth()
