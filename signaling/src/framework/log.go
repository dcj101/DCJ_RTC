package framework

import (
	"fmt"
	"math/rand"
	"time"

	"github.com/golang/glog"
)

type LogItem struct {
	field string
	value string
}

type timeItem struct {
	field     string
	beginTime int64
	endTime   int64
}

type ComLog struct {
	mainLog []LogItem
	timeLog []timeItem
}

func init() {
	rand.Seed(time.Now().Unix())
}

func GetLogIdInt32() int32 {
	return rand.Int31()
}

func (c *ComLog) TimeBegin(field string) {
	c.timeLog = append(c.timeLog, timeItem{field: field, beginTime: time.Now().UnixNano() / 1000})
}

func (c *ComLog) TimeEnd(field string) {
	for i, item := range c.timeLog {
		if item.field == field {
			c.timeLog[i].endTime = time.Now().UnixNano() / 1000
			return
		}
	}
}

func (c *ComLog) AddLogItem(field string, value string) {
	c.mainLog = append(c.mainLog, LogItem{field: field, value: value})
}

func (c *ComLog) GetMainLogPrefix() string {
	prefix := ""
	for _, item := range c.mainLog {
		prefix += fmt.Sprintf("%s[%s] ", item.field, item.value)
	}

	for _, item := range c.timeLog {
		prefix += fmt.Sprintf("%s[%.3f ms] ", item.field, float64(item.endTime-item.beginTime)/1000)
	}
	return prefix
}

func (c *ComLog) Debugf(format string, args ...interface{}) {
	totalLog := fmt.Sprintf("%s %s", c.GetMainLogPrefix(), format)
	glog.Debugf(totalLog, args...)
}

func (c *ComLog) Infof(format string, args ...interface{}) {
	totalLog := fmt.Sprintf("%s %s", c.GetMainLogPrefix(), format)
	glog.Infof(totalLog, args...)
}

func (c *ComLog) Errorf(format string, args ...interface{}) {
	totalLog := fmt.Sprintf("%s %s", c.GetMainLogPrefix(), format)
	glog.Errorf(totalLog, args...)
}

func (c *ComLog) Fatalf(format string, args ...interface{}) {
	totalLog := fmt.Sprintf("%s %s", c.GetMainLogPrefix(), format)
	glog.Fatalf(totalLog, args...)
}

func (c *ComLog) Warningf(format string, args ...interface{}) {
	totalLog := fmt.Sprintf("%s %s", c.GetMainLogPrefix(), format)
	glog.Warningf(totalLog, args...)
}
