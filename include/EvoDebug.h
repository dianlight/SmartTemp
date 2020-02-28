#pragma once
#include <Arduino.h>
#include <stdarg.h>
#include <exception>
#include <list>
/**
 * @file EvoDebug.h (HeaderOnly Function)
 * @author Lucio Tarantino
 * @brief Singleton Class for debugging
 * @version 0.1
 * @date 2020-02-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifdef EVODEBUG
    #define EVOD_MAX_PRINTF_LEN 256
    class EvoAppender {
        public:
            enum __attribute__((__packed__)) LEVEL {
                DEBUG,
                INFO,
                WARNING,
                ERROR
            };

             virtual void begin() = 0;
             virtual void end() = 0;
             virtual void displayMessage(LEVEL level,const char *fname,const char *mname,uint line, char* message) = 0;
    };

    #ifdef EVODEBUG_SERIAL 
        class SerialEvoAppender: public EvoAppender {
            public:
                void begin(){
                    Serial.println("SerialEvoAppender start");
                }

                void end(){
                    Serial.println("SerialEvoAppender stop");
                }

                void displayMessage(LEVEL level,const char *fname,const char *mname,uint line, char* message){
                    Serial.println(message);
                }
        };
    #endif

    class EvoDebug {
    private:
        EvoDebug(){
            #ifdef EVODEBUG_SERIAL
                EvoAppender *serialAppender = new SerialEvoAppender();
                addEvoAppender(serialAppender);
            #endif
        }

        std::list<EvoAppender*> appenders;

        ~EvoDebug(){
            std::list<EvoAppender*> :: iterator it; 
            for(it = appenders.begin(); it != appenders.end(); ++it) 
                 (*it)->end();
        }

        const char* LEVEL_NAME[5] = {"Debug","Info","Warning","Error"};

    public:
        EvoDebug(const EvoDebug&) = delete;
        EvoDebug& operator=(const EvoDebug &) = delete;
        EvoDebug(EvoDebug &&) = delete;
        EvoDebug & operator=(EvoDebug &&) = delete;

        static EvoDebug& instance(){
            static EvoDebug evoDebug;
            return evoDebug;
        }

        void addEvoAppender(EvoAppender *evoAppender){ appenders.push_back(evoAppender); evoAppender->begin(); }

        void message(EvoAppender::LEVEL level,const char *fileName,const char *functionName, uint line, const char *fmt, ...){
            va_list args;
            u8 p;
            std::list<EvoAppender*> :: iterator it; 
                for(it = appenders.begin(),p=0; it != appenders.end(); ++it,++p) { 
                    char* temp = new char[EVOD_MAX_PRINTF_LEN];
                    size_t lenh = snprintf(temp, EVOD_MAX_PRINTF_LEN, "[%s] {%s@%s|%d} ", LEVEL_NAME[level], functionName, fileName,line); 
                    va_start(args,fmt);
                    vsnprintf(&temp[lenh], EVOD_MAX_PRINTF_LEN - lenh, fmt, args);
                    va_end(args);    
                    (*it)->displayMessage(level,fileName, functionName, line, temp);
                    delete[] temp;
                }
        }
    }; 

    #define evoDebug EvoDebug::instance() 
    #define debugD(args...) EvoDebug::instance().message(EvoAppender::LEVEL::DEBUG,__FILE__ , __PRETTY_FUNCTION__ , __LINE__, args )
    #define debugI(args...) EvoDebug::instance().message(EvoAppender::LEVEL::INFO,__FILE__ , __PRETTY_FUNCTION__ , __LINE__, args )
    #define debugW(args...) EvoDebug::instance().message(EvoAppender::LEVEL::WARNING,__FILE__ , __PRETTY_FUNCTION__ , __LINE__, args )
    #define debugE(args...) EvoDebug::instance().message(EvoAppender::LEVEL::ERROR,__FILE__ , __PRETTY_FUNCTION__ , __LINE__, args )

#else
    #define debugD //
    #define debugI //
    #define debugW //
    #define debugE //
#endif 