#include "Gsender.h"


Gsender* Gsender::_instance = 0;
Gsender::Gsender(){}
Gsender* Gsender::Instance()
{
    if (_instance == 0) 
        _instance = new Gsender;
    return _instance;
}

void Gsender::setup(const char *smtpServer, const char * login, const char * password, const char * from, const char *to, int port){
  this->smtpServer = smtpServer;
  this->emailBase64Login = login;
  this->emailBase64Password = password;
  this->from = from;
  this->smtpPort = port;
  this->to = to;

  Serial.print(F("smtpServer: "));
  Serial.print(this->smtpServer);
  Serial.print(F(" login: "));
  Serial.print(this->emailBase64Login);
  Serial.print(F(" port: "));
  Serial.println(this->smtpPort);

}
Gsender* Gsender::Subject(const char* subject)
{
  delete [] _subject;
  _subject = new char[strlen(subject)+1];
  strcpy(_subject, subject);
  return _instance;
}
Gsender* Gsender::Subject(const String &subject)
{
  return Subject(subject.c_str());
}

bool Gsender::AwaitSMTPResponse(WiFiClientSecure &client, const String &resp, uint16_t timeOut)
{
  uint32_t ts = millis();
  while (!client.available())
  {
    if(millis() > (ts + timeOut)) {
      _error = "SMTP Response TIMEOUT!";
      return false;
    }
  }
  _serverResponce = client.readStringUntil('\n');
#if defined(GS_SERIAL_LOG_1) || defined(GS_SERIAL_LOG_2)
  Serial.println(_serverResponce);
#endif
  if (resp && _serverResponce.indexOf(resp) == -1) return false;
  return true;
}

String Gsender::getLastResponce()
{
  return _serverResponce;
}

const char* Gsender::getError()
{
  return _error;
}

bool Gsender::Send(const String &to, const String &message)
{
  WiFiClientSecure client;
#if defined(GS_SERIAL_LOG_2)
  Serial.print("Connecting to :");
  Serial.println(SMTP_SERVER);  
#endif
  if(!client.connect(smtpServer, smtpPort)) {
    _error = "Could not connect to mail server";
    return false;
  }
  if(!AwaitSMTPResponse(client, "220")) {
    _error = "Connection Error";
    return false;
  }

#if defined(GS_SERIAL_LOG_2)
  Serial.println("HELO friend:");
#endif
  client.println("HELO friend");
  if(!AwaitSMTPResponse(client, "250")){
    _error = "identification error";
    return false;
  }

#if defined(GS_SERIAL_LOG_2)
  Serial.println("AUTH LOGIN:");
#endif
  client.println("AUTH LOGIN");
  AwaitSMTPResponse(client);

#if defined(GS_SERIAL_LOG_2)
  Serial.println("EMAILBASE64_LOGIN:");
#endif
  client.println(emailBase64Login);
  AwaitSMTPResponse(client);

#if defined(GS_SERIAL_LOG_2)
  Serial.println("EMAILBASE64_PASSWORD:");
#endif
  client.println(emailBase64Password);
  if (!AwaitSMTPResponse(client, "235")) {
    _error = "SMTP AUTH error";
    return false;
  }
  
  String mailFrom = "MAIL FROM: <" + String(from) + '>';
#if defined(GS_SERIAL_LOG_2)
  Serial.println(mailFrom);
#endif
  client.println(mailFrom);
  AwaitSMTPResponse(client);

  String rcpt = "RCPT TO: <" + to + '>';
#if defined(GS_SERIAL_LOG_2)
  Serial.println(rcpt);
#endif
  client.println(rcpt);
  AwaitSMTPResponse(client);

#if defined(GS_SERIAL_LOG_2)
  Serial.println("DATA:");
#endif
  client.println("DATA");
  if(!AwaitSMTPResponse(client, "354")) {
    _error = "SMTP DATA error";
    return false;
  }
  
  client.println("From: <" + String(from) + '>');
  client.println("To: <" + to + '>');
  
  client.print(F("Subject: "));
  client.println(_subject);
  
  client.println(F("Mime-Version: 1.0"));
  client.println(F("Content-Type: text/html; charset=\"UTF-8\""));
  client.println(F("Content-Transfer-Encoding: 7bit"));
  client.println();
  String body = "<!DOCTYPE html><html lang=\"en\">" + message + "</html>";
  client.println(body);
  client.println(".");
  if (!AwaitSMTPResponse(client, "250")) {
    _error = "Sending message error";
    return false;
  }
  client.println(F("QUIT"));
  if (!AwaitSMTPResponse(client, "221")) {
    _error = "SMTP QUIT error";
    return false;
  }
  return true;
}
