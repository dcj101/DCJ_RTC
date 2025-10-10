package framework

import (
	"signaling/src/third_lib/goconfig"
)

type FrameworkConf struct {
	logDir      string
	logFile     string
	logLevel    string
	logToStderr bool

	httpPort         int
	httpStaticDir    string
	httpStaticPrefix string

	httpsPort int
	certFile  string
	keyFile   string
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

func (f *FrameworkConf) GetHttpPort() int {
	return f.httpPort
}

func (f *FrameworkConf) GetHttpStaticDir() string {
	return f.httpStaticDir
}

func (f *FrameworkConf) GetHttpStaticPrefix() string {
	return f.httpStaticPrefix
}

func (f *FrameworkConf) GetHttpsPort() int {
	return f.httpsPort
}

func (f *FrameworkConf) GetCertFile() string {
	return f.certFile
}

func (f *FrameworkConf) GetKeyFile() string {
	return f.keyFile
}

func GetHttpStaticDir() string {
	return gconf.httpStaticDir
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
	conf.httpPort = configFile.MustInt("http", "port")
	conf.httpStaticDir = configFile.MustValue("http", "staticDir")
	conf.httpStaticPrefix = configFile.MustValue("http", "staticPrefix")
	conf.httpsPort = configFile.MustInt("https", "port")
	conf.certFile = configFile.MustValue("https", "cert")
	conf.keyFile = configFile.MustValue("https", "key")
	return conf, nil
}
