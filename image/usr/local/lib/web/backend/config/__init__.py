class Default(object):
    DEBUG = True


class Development(Default):
    PHASE = 'development'


class Staging(Default):
    PHASE = 'staging'


class Production(Default):
    PHASE = 'production'
    DEBUG = False
