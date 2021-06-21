using System;
using log4net;
using log4net.Config;
using log4net.Appender;
using log4net.Layout;
using log4net.Repository.Hierarchy;

namespace ChatClient
{
    class Logger
    {
        ILog _log = null;

        public Logger() {

            var repository = LogManager.GetRepository();
            repository.Configured = true;
            // 콘솔 로그 패턴 설정
            var consoleAppender = new ConsoleAppender();
            consoleAppender.Name = "Console";
            // 로그 패턴
            consoleAppender.Layout = new PatternLayout("%d [%t] %-5p %c - %m%n");
            // 파일 로그 패턴 설정
            var rollingAppender = new RollingFileAppender();
            rollingAppender.Name = "RollingFile";
            // 시스템이 기동되면 파일을 추가해서 할 것인가? 새로 작성할 것인가?
            rollingAppender.AppendToFile = true;
            rollingAppender.DatePattern = "-yyyy-MM-dd";
            // 로그 파일 설정
            rollingAppender.File = @"d:\work\test.log";
            // 파일 단위는 날짜 단위인 것인가, 파일 사이즈인가?
            rollingAppender.RollingStyle = RollingFileAppender.RollingMode.Date;
            // 로그 패턴
            rollingAppender.Layout = new PatternLayout("%d [%t] %-5p %c - %m%n");
            var hierarchy = (Hierarchy)repository;
            hierarchy.Root.AddAppender(consoleAppender);
            hierarchy.Root.AddAppender(rollingAppender);
            rollingAppender.ActivateOptions();
            // 로그 출력 설정 All 이면 모든 설정이 되고 Info 이면 최하 레벨 Info 위가 설정됩니다.
            hierarchy.Root.Level = log4net.Core.Level.All;
            _log = LogManager.GetLogger(this.GetType());
        }
        
        public void Info(object message)
        {
            _log.Info(message);
        }

        public void Debug(object message)
        {
            _log.Debug(message);
        }

        public void Error(object message)
        {
            _log.Error(message);
        }

        public void Fatal(object message)
        {
            _log.Fatal(message);
        }
    }
}
