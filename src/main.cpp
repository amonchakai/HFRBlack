/*
 * Copyright (c) 2011-2013 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bb/cascades/Application>

#include <QLocale>
#include <QTranslator>
#include "applicationui.hpp"

#include <Qt/qdeclarativedebug.h>

#include "Settings.hpp"

using namespace bb::cascades;

void debugOutputMessages(QtMsgType type, const char *msg) {

   if(!Settings::getLogEnabledUI()) {
        switch (type) {
             case QtDebugMsg:
                 fprintf(stderr, "%s\n", msg);
                 break;
             case QtWarningMsg:
                 fprintf(stderr, "%s\n", msg);
                 break;
             case QtCriticalMsg:
                 fprintf(stderr, "%s\n", msg);
                 break;
             case QtFatalMsg:
                 fprintf(stderr, "%s\n", msg);
                 abort();
        }
   } else {

       QString directory = QDir::homePath() + QLatin1String("/ApplicationData");
       if (!QFile::exists(directory)) {
           QDir dir;
           dir.mkpath(directory);
       }

       QFile file(directory + "/Logs.txt");
       if (!file.open(QIODevice::Append)) {
           return;
       }
       QTextStream stream(&file);

       switch (type) {
           case QtDebugMsg:
               stream << "<div class=\"debug\">[DEBUG]"  <<  msg << "</div>";
               break;
           case QtWarningMsg:
               stream << "<div class=\"warning\">[WARNING]"  <<  msg << "</div>";
               break;
           case QtCriticalMsg:
               stream << "<div class=\"critical\">[CRITICAL]"  <<  msg << "</div>";
               break;
           case QtFatalMsg:
               stream << "<div class=\"fatal\">[FATAL]"  <<  msg << "</div>";
               file.close();
               abort();
       }

       file.close();
   }

 }


Q_DECL_EXPORT int main(int argc, char **argv)
{
    qInstallMsgHandler(debugOutputMessages);
	Settings::loadSettings();

	switch(Settings::themeValue()) {
		case 1:
			qputenv("CASCADES_THEME", "Bright");
			break;
		case 2:
			qputenv("CASCADES_THEME", "Dark");
			break;
	}


    Application app(argc, argv);


    // Create the Application UI object, this is where the main.qml file
    // is loaded and the application scene is set.
    new ApplicationUI(&app);

    // Enter the application main event loop.
    return Application::exec();
}
