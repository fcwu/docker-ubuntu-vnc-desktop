from __future__ import (
    absolute_import, division, print_function, with_statement
)
from functools import wraps
import logging
from flask import jsonify


log = logging.getLogger()


class PermissionDenied(Exception):
    pass


class BadRequest(Exception):
    pass


def httperror(f):
    @wraps(f)
    def func(*args, **kwargs):
        result = {
            'code': 400,
            'errorMessage': '',
        }
        try:
            return f(*args, **kwargs)
        except PermissionDenied as e:
            result['code'] = 403
            result['errorMessage'] = str(e)
        except BadRequest as e:
            result['code'] = 400
            result['errorMessage'] = str(e)
        except Exception as e:
            logging.exception(e)
            result['code'] = 500
            result['errorMessage'] = str(e)
        return jsonify(result)
    return func
