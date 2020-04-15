class Default(object):
    DEBUG = True


class Development(Default):
    PHASE = 'development'


class Production(Default):
    PHASE = 'production'
    DEBUG = False
