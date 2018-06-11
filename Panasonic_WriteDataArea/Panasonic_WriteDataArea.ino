/*
 * Rutina de escritura Protocolo  Mewtocol  WD Write data area, creada  para Equipos Industriales Panasonic Principalmente PLC'S y HMI' entre otros dispositivos
 * Write Routine Protocol Mewtocol WD Write data area, created for Panasonic Industrial Equipment Mainly PLC'S and HMI 'among other devices
 * 
 *  Created by PDAControl
 *   Write DT'S Registers
 *   
 *  More info      : http://pdacontrolen.com
 *  Mas informacion: http://pdacontroles.com
 *  Channel  Youtube https://www.youtube.com/c/JhonValenciaPDAcontrol/videos   
 *  
 *  
 *  Routine write-only in registers - Rutina solo escritura en registros
 *   DT = Data Registers
 *   LD = Link Data Registers
 *   FL = File Registers
 *  

*/

void setup() {
 

pinMode(13, OUTPUT);              // ON= "Communication OK"  OFF = "Communication ERROR or OFF PLC "
Serial.begin(9600);               /// Debug serial
Serial1.begin(9600,SERIAL_8O1);   /// Communication PLC Panasonic

}




//                  Funtion  ADDCERO
//////////////////////////////////////////////////////////////////
///Esta funcion rellena con ceros a la izquierda dependiendo del tamaño requerido
///This function fills with leading zeros depending on the required size       example   in(10,5) = 10000  add 3 zeros  

String addcero(String value ,int Max  )
{
int Tam = value.length() ;
String ceroadd="";

for (int i=0; i < (Max - Tam) ; i++){ 
   ceroadd+="0";
}
value=ceroadd+value;
return value;

}
////////////////////////////////////////////////////////////////////////



////////////////////Funtion WD_MEWTOCOL ////////////////////////////////////////////////////////////////////////////////

// EXAMPLE  DT300 = 100    //  Value   100 =  64 hex ->> invertir   L (64)     H (100)
//  This function takes input data and creates the entire write frame. Mewtocol performs the following:
// Header + REGISTRY TYPE + Address Register + Address Register + Conversion of int value to HEX to value in ASCII + Checksum (BCC)
//   %EE#WD-----------D ------------00300-------------00300----------------64 00-------------------------------------------53

//  Ejemplo  DT300 = 100    //  Valor   100 =  64 hex ->> invertir   L (64)     H (100)
//  Esta funcion toma datos de entrada y crea toda la trama de escritura Mewtocol realiza lo siguiente:
///  Encabezado + TIPO REGISTRO  + Direccion Registro + Direccion Registro  + Conversion de Valor int a HEX a valor en ASCII + Checksum (BCC) 
//   %EE#WD-----------D ------------------00300-----------------00300----------------64 00--------------------------------------53                              

void WD_Mewtocol(String RegistroDT, int valor )
{
  
 //String Trama ="%EE#WDD0030000300640053";   // test 
String Trama="";                                                                                   

////////////// Header - Encabezado   /////////////////////////
String DT = "D";  /// Data Registers
String LD = "L";  /// Link Data Registers
String FL = "F";  /// File Registers
String Datacode = DT;  ///// DT Registers
String Encabezado="%EE#WD"+Datacode;               
///////////////////////////////////////////////////////////////

/////////////////////////TIPO REGISTRO//////////////////////////////////////
String DireccionDT = RegistroDT.substring(2,6) ;  /// Recorta direccion DT  ej:  DT1000  ---> ////   Cut DT direction eg: DT1000 ---> 1000
DireccionDT = addcero(DireccionDT,5);            /// agrega relleno ceros ala izquierda   /// Add padding zeros left
///////////////////////////////////////////////////////////////

//////////////////////High and Low Value, conversion int to HEX and String         //////////////////
//////////////////////Parte Alta y Baja de Valor , conversion int a HEX y String  ///////////////////
byte hi, lo;  // 8 bits C/U         
hi = highByte(valor); 
lo = lowByte(valor);   

String valorH=String(hi, HEX);
String valorL=String(lo, HEX);

valorL.toUpperCase();
valorL = addcero(valorL,2);   /// agrega relleno ceros ala izquierda   /// Add padding zeros left
//Serial.println(valorL);

valorH.toUpperCase();
valorH = addcero(valorH,2);  /// agrega relleno ceros ala izquierda   /// Add padding zeros left
//Serial.println(valorH);
//Serial.println(valorL+valorH);
/////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////     Frame Concatenation   ////////////////////////////////////
////////////    Concatenación de trama ////////////////////////////////////
Trama = (Encabezado + DireccionDT + DireccionDT + valorL + valorH );
Trama.toUpperCase();   /////////CONVERTING String to upper case   -  CONVERSIÓN DE String a mayúsculas
///////////////////////////////////////////////////////////////////////////



/////////////////////////CALCULO BCC (Checksum de seguridad)////////////////////////////////////////////////
///////////////////////// BCC CALCULO  (Security Checksum)//////////////////////////////////////////////////
/* 
 *  El BCC es un código que realiza la verificación de errores de paridad horizontal para Mejorar la fiabilidad de los datos que se envían.
 *  El BCC utiliza un OR exclusivo desde el encabezado (%) hasta el carácter final del Texto y convierte los datos de 8 bits en un código ASCII de 2 caracteres.
 * 
 * The BCC is a code that performs horizontal parity error checking to Improve the reliability of the data being sent.
 * The BCC uses an exclusive OR from the header (%) to the final character of the Text and converts the 8-bit data into a 2-character ASCII code. * 
 */
 
int  TramaTam =Trama.length();             // Longitud  Trama  //  Length Frame
byte tram_arreglo [TramaTam];              //  Array Bytes
Trama.getBytes(tram_arreglo,TramaTam+1);   // almacena Trama en BYTES Array // Stores Frame in BYTES Array     

byte bcc = tram_arreglo[0]; 
String BCC;

 for (int i=1; i <= (TramaTam); i++){ 
     bcc ^= tram_arreglo[i];                      /// exclusive or     
 }
BCC =String(bcc, HEX);          ////// BCC to String    
////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////Concatenate BCC to frame///////////////////////////////////////////////////////////
Trama = Trama + BCC ;
Trama.toUpperCase();
///////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////Sends Frame for Serial1 Port "Arduino Mega"/////////////////
Serial1.print(Trama+"\r");
///////////////////////////////////////////////////////////////////////////


/////////////////////////Confirmation response from PLC  //////////////////////////////////////////////////
/////////////////////////Respuesta de confirmacion desde PLC////////////////////////////////////////////////+
///////////////////      %EE$WD12 = OK


String respuesta =(Serial1.readString());
String Error = respuesta;
Serial1.setTimeout(200);                //////Timeout  
String Validacion = respuesta.substring(0,6);
     
      if(Validacion =="%EE$WD")        //Normal response (Write successful)
      {
       
       Serial.print(RegistroDT);
       Serial.println(" = OK");
       digitalWrite(13, HIGH); 
      }
      else
      { 
        Serial.print(RegistroDT);
        Serial.print(" = FAIL !!!! Error = "); 
        Serial.println(Error);
        digitalWrite(13, LOW); 
      }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


      
}



void loop() {

 int adc = analogRead(A0);
  Serial.println(adc);
   delay(50);
  
  WD_Mewtocol("DT100",adc); 
   delay(50);
  WD_Mewtocol("DT101",adc);    
   delay(50);
  WD_Mewtocol("DT102",adc);    
   delay(50);



}
