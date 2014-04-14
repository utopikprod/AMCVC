/*
Get MIDI message and transform them into Hexa numbers driving a 12 bits DAC
By Utopikprod
2014
*/

#include <MIDI.h>
#include <SPI.h>

//MIDI channel used
const int MIDIChannel = 3;

//CV VCO tuning
const int VCO_CV_Offset = 0;
const float VCO_CV_step = 68.25;
const int MIDI_Min_Note = 24;
const int MIDI_Max_Note = 84;
const int MIDI_Start_Offset = 24;

//const IO variables
const int midiInput_Pin = 2;
//const int DACInputPin = 3;
const int led_Pin = 6;

//MCP4922 DAC control
const int CS_Pin = 10;
const int LDAC_Pin = 11;

const int VCO_DAC_ID = 0;
const int VCF_DAC_ID = 1;

const int VCO_Gate_Pin = 9;
const int VCF_Gate_Pin = 17;

byte LSBytes = 0x0;
byte MSBytes = 0x0;

int type, note, velocity, channel, d1, d2;

//x0x heart specific control
const byte accentCC = 0x64;
const byte slideCC = 0x66;
const byte triggerLimit = 0x64;

//MIDI specific table
const byte noteOff = 0x80;
const byte noteOn = 0x90;
const byte aftertouch = 0xA0;
const byte cc = 0xB0;
const byte patchRange = 0xC0;
const byte channelPressure = 0xD0;
const byte pitchBend = 0xE0;
const byte sysexStart = 0xF0;

const byte sysexStop = 0xF7;
const byte clock = 0xF8;
const byte startSeq = 0xFA;
const byte contSeq = 0xFB;
const byte stopSeq = 0xFC;

HardwareSerial Uart = HardwareSerial();

void setup()
{
  
 MIDI.begin(MIDI_CHANNEL_OMNI);
 
 SPI.begin();
 SPI.setBitOrder(MSBFIRST);
 SPI.setDataMode(SPI_MODE0);
 
 Uart.begin(31250);
 Serial.begin(57600);
 
 pinMode(PIN_D2, INPUT);
 pinMode(PIN_D3, OUTPUT);
 
 pinMode(led_Pin,OUTPUT);
 pinMode(midiInput_Pin, INPUT);
 
 pinMode(CS_Pin, OUTPUT);
 pinMode(LDAC_Pin, OUTPUT);
 
 pinMode(VCO_Gate_Pin, OUTPUT);
 pinMode(VCF_Gate_Pin, OUTPUT);
 
 digitalWrite(led_Pin,HIGH);
}

void WriteDAC()
{ 
  digitalWrite(LDAC_Pin,HIGH);
  digitalWrite(CS_Pin,LOW);//Active LSB
  
  SPI.transfer(MSBytes);
  SPI.transfer(LSBytes);
  
  digitalWrite(CS_Pin,HIGH);//Deactive LSB
  digitalWrite(LDAC_Pin,LOW);
}

void ProcessData(int messageType)
{
  //Midi Off Mess
  if( messageType == 0 )
  {
    //Set gate voltage to 0V
    digitalWrite(VCO_Gate_Pin,LOW);
  }
  else
  {
    //Midi On Mess
    if(messageType == 1)
    {
      blinkLED();
      //Set gate voltage to 5V
      digitalWrite(VCO_Gate_Pin,HIGH);
                
      //Set vco voltage
      note += VCO_CV_Offset;
      note = constrain(note, MIDI_Min_Note, MIDI_Max_Note);
      int convertedVal = (int)((float)(note-MIDI_Start_Offset) * VCO_CV_step);
      
      LSBytes = lowByte(convertedVal);
      MSBytes = highByte(convertedVal);
      MSBytes = MSBytes | 0b00110000;
      
      Serial.print("LSBytes : ");
      Serial.print(LSBytes,BIN);
      Serial.print(" / ");
      Serial.println(LSBytes,HEX);
      
      Serial.print("MSBytes : ");
      Serial.print(MSBytes,BIN);
      Serial.print(" / ");
      Serial.println(MSBytes,HEX);
      
      WriteDAC();
    }
    else
    {
      //Midi CC Mess
      if(messageType == 2)
      {
        if(note==accentCC && velocity>triggerLimit)
        {
           //Set On digital out for accent
        }
        
        if(note==slideCC && velocity>triggerLimit)
        {
           //Set On digital out for slide
        }
      }
    }
  }
}

void blinkLED()
{
   digitalWrite(led_Pin,LOW);
   delay(25);
   digitalWrite(led_Pin,HIGH);
}

void loop()
{
  if( MIDI.read() )
  {
    byte type = MIDI.getType();
    switch (type) {
      case NoteOn:
        note = MIDI.getData1();
        velocity = MIDI.getData2();
        channel = MIDI.getChannel();
        if (velocity > 0) 
        {
          Serial.println(String("Note On:  ch=") + channel + ", note=" + note + ", velocity=" + velocity);
          ProcessData(1);
        } 
        else 
        {
          //Serial.println(String("Note Off: ch=") + channel + ", note=" + note);
          ProcessData(0);
        }
        break;
      case NoteOff:
        note = MIDI.getData1();
        velocity = MIDI.getData2();
        channel = MIDI.getChannel();
        //Serial.println(String("Note Off: ch=") + channel + ", note=" + note + ", velocity=" + velocity);
        ProcessData(0);
        break;
      default:
        d1 = MIDI.getData1();
        d2 = MIDI.getData2();
        //Serial.println(String("Message, type=") + type + ", data = " + d1 + " " + d2);
    }
  }
}
