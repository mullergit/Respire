String readFile2(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
}
void writeFile2(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

//************************************************************8
void createFile(void){
  File wFile; 
  if(SPIFFS.exists("/log.txt")){
    Serial.println("Arquivo ja existe!");
  } else {
    Serial.println("Criando o arquivo...");
    wFile = SPIFFS.open("/log.txt","w+"); 
    if(!wFile){
      Serial.println("Erro ao criar arquivo!");
    } else {
      Serial.println("Arquivo criado com sucesso!");
    }
  }
  wFile.close();
}
void writeFile(String msg) {
  int w;
  File rFile = SPIFFS.open("/log.txt","a+");  
  Serial.println();
  if(!rFile){
    Serial.println("Erro ao abrir arquivo!");
  } else {
    //w = rFile.write(msg, bufLen);
    rFile.println();
    Serial.print("\t write: ");Serial.print(w);Serial.println(" Bytes ");
  }
  rFile.close();  
}

void readFile1(void) { 
  int w; 
  Serial.print("\t fazendo leirura 1 ..... ");
  File rFile = SPIFFS.open("/log.txt","r"); 
  if(!rFile){
    Serial.println("Erro ao abrir arquivo!");
  } else {
    //w = rFile.read(codes.buf, bufLen);     
    Serial.print("\t read: ");Serial.print(w);Serial.println(" Bytes ");
  }   
  rFile.close();
}

void openFS(void){
  if(!SPIFFS.begin()){
    Serial.println("Erro ao abrir o sistema de arquivos");
  } else {
    Serial.println("Sistema de arquivos aberto com sucesso!");
  }
}
