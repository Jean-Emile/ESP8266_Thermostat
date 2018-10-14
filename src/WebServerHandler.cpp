//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "WebServerHandler.h"



WebServerHandler::WebServerHandler() {
    server = new ESP8266WebServer(80);
}
WebServerHandler::~WebServerHandler() {
    delete server;
}

String WebServerHandler::getContentType(String filename){
    if(filename.endsWith(".htm")) return "text/html";
    else if(filename.endsWith(".html")) return "text/html";
    else if(filename.endsWith(".css")) return "text/css";
    else if(filename.endsWith(".js")) return "application/javascript";
    else if(filename.endsWith(".png")) return "image/png";
    else if(filename.endsWith(".gif")) return "image/gif";
    else if(filename.endsWith(".jpg")) return "image/jpeg";
    else if(filename.endsWith(".ico")) return "image/x-icon";
    else if(filename.endsWith(".xml")) return "text/xml";
    else if(filename.endsWith(".pdf")) return "application/x-pdf";
    else if(filename.endsWith(".zip")) return "application/x-zip";
    else if(filename.endsWith(".gz")) return "application/x-gzip";
    return "text/plain";
}

bool WebServerHandler::handleFileRead(String path) { // send the right file to the client (if it exists)
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
    String contentType = getContentType(path);             // Get the MIME type
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
            path += ".gz";                                         // Use the compressed verion
        File file = SPIFFS.open(path, "r");                    // Open the file
        size_t sent = server->streamFile(file, contentType);    // Send it to the client
        file.close();                                          // Close the file again
        Serial.println(String("\tSent file: ") + path);
        return true;
    }
    Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
    return false;
}

void WebServerHandler::handleNotFound(){ // if the requested file or page doesn't exist, return a 404 not found error
    if(!handleFileRead(server->uri())){          // check if the file exists in the flash memory (SPIFFS), if so, send it
        server->send(404, "text/plain", "404: File Not Found");
    }
}

void WebServerHandler::handleFileUpload(){ // upload a new file to the SPIFFS
    HTTPUpload& upload = server->upload();
    String path;
    if(upload.status == UPLOAD_FILE_START){
        path = upload.filename;
        if(!path.startsWith("/")) path = "/"+path;
        if(!path.endsWith(".gz")) {                          // The file server always prefers a compressed version of a file
            String pathWithGz = path+".gz";                    // So if an uploaded file is not compressed, the existing compressed
            if(SPIFFS.exists(pathWithGz))                      // version of that file must be deleted (if it exists)
                SPIFFS.remove(pathWithGz);
        }
        Serial.print("handleFileUpload Name: "); Serial.println(path);
        fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
        path = String();
    } else if(upload.status == UPLOAD_FILE_WRITE){
        if(fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    } else if(upload.status == UPLOAD_FILE_END){
        if(fsUploadFile) {                                    // If the file was successfully created
            fsUploadFile.close();                               // Close the file again
            Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
            server->sendHeader("Location","/success.html");      // Redirect the client to the success page
            server->send(303);
        } else {
            server->send(500, "text/plain", "500: couldn't create file");
        }
    }
}




void WebServerHandler::setup() { // Start a HTTP server with a file read handler and an upload handler

    this->server->on("/ota.html",  HTTP_POST, [&]() {  // If a POST request is sent to the /ota.html address,
         server->send(200, "text/plain", "");
    }, std::bind(&WebServerHandler::handleFileUpload, this));                       // go to 'handleFileUpload'

    // if someone requests any other file or page, go to function 'handleNotFound'
    // and check if the file exists
    server->onNotFound(std::bind(&WebServerHandler::handleNotFound, this));

    this->server->begin();                             // start the HTTP server
    Serial.println("HTTP server started.");
}

void WebServerHandler::update() {
    server->handleClient();
}
