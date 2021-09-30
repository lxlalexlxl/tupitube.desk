/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
 *                                                                         *
 *   KTooN's versions:                                                     *
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "tuppapagayoimporter.h" 

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

TupPapagayoImporter::TupPapagayoImporter(const QString &file, const QSize &projectSize, const QString &extension, 
                                         int frameIndex) : QObject()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoImporter::TupPapagayoImporter()]";
    #endif

    initFrame = frameIndex;
    int framesTotal = 0;
    framesCount = 0;
    isValid = true;
    QFile input(file);

    QFileInfo info(file);
    QString name = info.fileName().toLower();
    lipsync = new TupLipSync();
    lipsync->setLipSyncName(name);
    lipsync->setInitFrame(initFrame);
    lipsync->setPicsExtension(extension);

    if (input.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&input);
        int i = 0;
        int voicesNumber = 0;
        QString line;
        while (i < 5) {
               line = stream.readLine(); 
               switch (i) {
                   case 0:
                   {
                       if (!line.startsWith("lipsync version")) {
                           isValid = false;
                           return;
                       }
                   } 
                   break;
                   case 2:
                   {
                       // FPS 
                       fps = line.trimmed().toInt();
                       lipsync->setFPS(fps);
                   }
                   break;
                   case 3:
                   {
                       // Frames Total
                       framesTotal = line.trimmed().toInt();
                   }
                   break;
                   case 4:
                   {
                       // Total of voices 
                       voicesNumber = line.trimmed().toInt();
                   }
                   break;
               }
               i++;
        }

        int x = projectSize.width() / (voicesNumber+1);
        int y = projectSize.height() / 2;

        for(int j=1; j<=voicesNumber; j++) {
            TupVoice *voice = new TupVoice(); 
            x = x*j;

            QPointF point(x, y);
            voice->setMouthPos(point);
            voice->setVoiceTitle(stream.readLine().trimmed());
            voice->setText(stream.readLine().trimmed());
            int numPhrases = stream.readLine().toInt();
            int numPhonemes = 0;
            int numWords;
            int firstFrame = 0;
            int lastFrame = 0;

            for (int p = 0; p < numPhrases; p++) {
                 line = stream.readLine().trimmed();
                 int phInitFrame = stream.readLine().trimmed().toInt();

                 if (p == 0)
                     phInitFrame = 0;
                 line = stream.readLine().trimmed(); // Skipping line
                 TupPhrase *phrase = new TupPhrase(phInitFrame);
                 numWords = stream.readLine().toInt();

                 for (int w = 0; w < numWords; w++) {
                      QString str = stream.readLine().trimmed();
                      QStringList strList = str.split(' ', Qt::SkipEmptyParts);
                      QString strWord; 
                      TupWord *word = 0;
                      if (strList.size() >= 4) {
                          strWord = strList.at(0);   
                          firstFrame = strList.at(1).toInt();
                          word = new TupWord(firstFrame);
                          lastFrame = strList.at(2).toInt();
                          word->setEndFrame(lastFrame);
                          numPhonemes = strList.at(3).toInt();
                          #ifdef TUP_DEBUG
                              if (numPhonemes == 0) {
                                  qDebug() << "[TupPapagayoImporter::TupPapagayoImporter()] - Warning: Word \""
                                           <<  strWord << "\" has NO phonemes associated! :(";
                              }
                          #endif
                      } else {
                          #ifdef TUP_DEBUG
                              qDebug() << "[TupPapagayoImporter::TupPapagayoImporter()] - Warning: Missing parameters -> " << str;
                          #endif
                      }
                      QList<int> frames;
                      QList<QString> blocks;
                      for (int ph = 0; ph < numPhonemes; ph++) {
                           str = stream.readLine().trimmed();
                           QStringList strList = str.split(' ', Qt::SkipEmptyParts);
                           if (strList.size() >= 2) {
                               frames << strList.at(0).toInt();
                               blocks << strList.at(1).toLower();
                           }
                      } // for ph

                      for (int ph = 0; ph < numPhonemes-1; ph++) {
                           int total = frames.at(ph+1) - frames.at(ph);
                           for (int i=0; i<total; i++) {
                                TupPhoneme *phoneme = new TupPhoneme(blocks.at(ph), point);
                                word->addPhoneme(phoneme);
                           }
                      } // for ph

                      if (!frames.isEmpty()) {
                          int total = (lastFrame - frames.at(numPhonemes-1)) + 1;
                          for (int i=0; i<total; i++) {
                               TupPhoneme *phoneme = new TupPhoneme(blocks.at(numPhonemes-1), point);
                               word->addPhoneme(phoneme);
                          }
                      } else {
                          #ifdef TUP_DEBUG
                              qDebug() << "[TupPapagayoImporter::TupPapagayoImporter()] - Fatal Error: frames size is less than numPhonemes -> "
                                       << frames.count() << " < " << numPhonemes;
                          #endif
                      }

                      // Checking frames count consistency
                      if (w == numWords - 1) { // If last word
                          if (lastFrame > 0)
                              framesCount = lastFrame;

                          // If lip-sync is longer that latest word, fill empty frames with rest mouth
                          if (lastFrame < framesTotal) {
                              int distance = framesTotal - lastFrame;
                              for (int i=0; i<distance; i++) {
                                   TupPhoneme *phoneme = new TupPhoneme("rest", point);
                                   word->addPhoneme(phoneme);
                              }
                              framesCount += distance;
                          }
                      }
                      phrase->addWord(word);
                 } // for w
                 phrase->setEndFrame(framesCount);
                 voice->addPhrase(phrase); 
            }
            lipsync->addVoice(voice);
        }

        framesCount++;
        if (framesTotal > framesCount) {
            framesCount = framesTotal;
            lipsync->setFramesCount(framesTotal);
        } else {
            lipsync->setFramesCount(framesCount);
        }

        lipsync->verifyStructure();
    } else {
        isValid = false;
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoImporter::TupPapagayoImporter()] - Fatal Error: Insufficient permissions to load file! -> " << file;
        #endif
        return;
    }

    input.close();
}

TupPapagayoImporter::~TupPapagayoImporter()
{
}

void TupPapagayoImporter::setSoundFile(const QString &soundFile)
{
    lipsync->setSoundFile(soundFile);
}

bool TupPapagayoImporter::fileIsValid()
{
    return isValid;
}

int TupPapagayoImporter::getFrameCount()
{
    return framesCount;
}

int TupPapagayoImporter::getFps()
{
    return fps;
}

int TupPapagayoImporter::getInitFrame()
{
    return initFrame;
}

QString TupPapagayoImporter::toString() const
{
    QDomDocument document;
    QDomElement root = lipsync->toXml(document);

    QString xml;
    {
      QTextStream ts(&xml);
      ts << root;
    }

    return xml;
}
