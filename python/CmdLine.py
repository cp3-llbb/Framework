from FWCore.ParameterSet.VarParsing import VarParsing

class CmdLine(object):
    """
    Parse the command line for options. Supported options are:
        - 'globalTag': the global tag to use
        - 'era': '25ns' or '50ns' (for 2015 data) or '2016' or '2017' (default)
        - 'process': The process name used in the MiniAOD generation
        - 'runOnData': 1 if running on data, 0 otherwise
        - 'hltProcessName': the process name used when running the HLT

    Can be customized from a config file (or better, subclass) by passing
    an override and/or a new default options dictionary to the constructor,
    or by calling override(option=value, ...) and
    changeDefaults(option=value, ...) directly, depending on depending on
    whether the given value should be used is none is passed on the command
    line, or if it should be used regardless of that.
    Options can be added using options.register.
    The registerOptions method (called from the constructor) can be
    overridden by subclasses, e.g.
    >>> from cp3_llbb.Framework.CmdLine import CmdLine
    >>> class MyCmdLine(CmdLine):
    >>>     def __init__(self, override=None, defaults=None):
    >>>         super(MyCmdLine, self).__init__(override=override, defaults=defaults)
    >>>     def registerOptions(self):
    >>>         super(MyCmdLine, self).registerOptions()
    >>>         self.options.register(...) ## see FWCore.ParameterSet.VarParsing
    >>>         ## optional (if it makes sense to do this at the class level)
    >>>         self.changeDefaults(option=value, ...)
    >>>         self.override(option=value, ...)
    >>>
    >>> options = MyCmdLine(defaults=dict(era="2016"))
    >>> isData = options.runOnData ## triggers parsing of options

    Note: parsing of the arguments happens as soon as an option is accessed,
    so no more changes can be made after that.
    """
    def __init__(self, override=None, defaults=None):
        self._overridden = override if override is not None else dict()
        self.options = VarParsing()
        self._parsed = False
        self.registerOptions()
        if defaults is not None:
            self.changeDefaults(**defaults)

    def override(self, **kwargs):
        """ Override value for given keys

        (these take precedence over command line values) """
        self._overridden.update(kwargs)

    def changeDefaults(self, **kwargs):
        """ Change default values for given keys

        (these are only used if nothing is specified on the command line) """
        for k,v in kwargs.iteritems():
            if ( not self._parsed ) or ( not self.options._beenSet.get(k, False) ):
                self.options.setDefault(k, v)

    def registerOptions(self):
        """ register options (called at the end of construction - can be overridden) """
        if self._parsed:
            raise AssertionError("Cannot register options after parsing")

        self.options.register('runOnData',
                -1,
                VarParsing.multiplicity.singleton,
                VarParsing.varType.int,
                'If running over MC (0) or data (1)')

        self.options.register('globalTag',
                '',
                VarParsing.multiplicity.singleton,
                VarParsing.varType.string,
                'The globaltag to use')

        self.options.register('era',
                '2017',
                VarParsing.multiplicity.singleton,
                VarParsing.varType.string,
                'Era of the dataset')

        self.options.register('process',
                '',
                VarParsing.multiplicity.singleton,
                VarParsing.varType.string,
                'Process name of the MiniAOD production.')

        self.options.register('hltProcessName',
                'HLT',
                VarParsing.multiplicity.singleton,
                VarParsing.varType.string,
                'The HLT processName to use')

    def _ensureParsed(self):
        if not self._parsed:
            self._parsed = True
            self.options.parseArguments()

    def __getattr__(self, name):
        """
        Forward call to the options object
        """
        self._ensureParsed()
        if name in self._overridden:
            return self._overridden[name]
        else:
            return getattr(self.options, name)

    @property
    def runOnData(self):
        self._ensureParsed()
        return ( self.options.runOnData == 1 )
    @property
    def era(self):
        self._ensureParsed()
        assert self.options.era in ("25ns", "50ns", "2016", "2017")
        from Configuration.StandardSequences.Eras import eras
        return getattr(eras, "Run2_{}".format(self.options.era))
