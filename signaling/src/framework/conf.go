package framework

import "signaling/src/third_lib/goconfig"

type FrameworkConf struct {
	logDir      string
	logFile     string
	logLevel    string
	logToStderr bool
}

var configFile *goconfig.ConfigFile

func (f *FrameworkConf) GetLogDir() string {
	return f.logDir
}

func (f *FrameworkConf) GetLogFile() string {
	return f.logFile
}

func (f *FrameworkConf) GetLogLevel() string {
	return f.logLevel
}

func (f *FrameworkConf) GetLogToStderr() bool {
	return f.logToStderr
}

func loadConfig(confFilePath string) (*FrameworkConf, error) {
	var err error
	configFile, err = goconfig.LoadConfigFile(confFilePath)
	if err != nil {
		return nil, err
	}

	conf := &FrameworkConf{}
	conf.logDir = configFile.MustValue("log", "logDir")
	conf.logFile = configFile.MustValue("log", "logFile")
	conf.logLevel = configFile.MustValue("log", "logLevel")
	conf.logToStderr = configFile.MustBool("log", "logToStderr")
	return conf, nil
}
