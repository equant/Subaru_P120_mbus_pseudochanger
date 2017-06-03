//#include <AdvancedRemote.h>
#include <MBusPanasonic.h>

//#define IPOD_SERIAL_DEBUG
#define DEBUG_MODE
//#define DISABLE_RELAY

//playback state definitions
#define IPOD_ACTIVE 1
#define AUX_ACTIVE  2
#define MBUS_DELAY 7

//#define mBusIn  PIN_D1
//#define mBusOut PIN_D0
#define mBusIn  13
#define mBusOut 12

//pin definitions
//static const byte SENSE_IPOD  = PIN_F4;
//static const byte LED_AUX     = PIN_C6;
//static const byte LED_IPOD    = PIN_C7;
//static const byte RELAY_AUX   = PIN_B0;
//static const byte TOGGLE_AUX  = PIN_D5;
//static const byte TOGGLE_IPOD = PIN_D4;

static const byte PIN_D6 = 6;

static const byte LED_AUX     = 2;
static const byte LED_IPOD    = 3;
static const byte TOGGLE_AUX  = 5;
static const byte TOGGLE_IPOD = 4;

byte activeInput;

boolean ipodAvailable = true;

MBus mBus(mBusIn, mBusOut);

//HardwareSerial uart = HardwareSerial();
//AdvancedRemote ipod = AdvancedRemote(Serial1);

byte playbackStatus = STOPPED;
boolean updateDisplay = false;

unsigned long plistindex = 1;
unsigned long trackindex = 1;

unsigned long playtime = 0;
unsigned long plistcount = 0;
unsigned long trackcount = 0;

uint64_t mbusMsg = 0ULL;
uint64_t nextUpdate = 0;
uint64_t prevPing   = 0;

uint32_t tmp = 0;

/// iPod AdvancedRemote callbacks ////////////////////////////////////
/*
void shuffleModeHandler(AdvancedRemote::ShuffleMode mode){
  switch(mode){
  case AdvancedRemote::SHUFFLE_MODE_OFF:
    Serial.println("shuffle off");
    playbackStatus &= ~SHUFFLE_MASK;
    break;
  case AdvancedRemote::SHUFFLE_MODE_SONGS:
    Serial.println("shuffle songs");
    playbackStatus =  ((~SHUFFLE_MASK)&playbackStatus)| SHUFFLE_SONGS;
    break;
  case AdvancedRemote::SHUFFLE_MODE_ALBUMS:
    Serial.println("shuffle off");
    playbackStatus =  ((~SHUFFLE_MASK)&playbackStatus)| SHUFFLE_ALBUMS;
    break;
  }
}

void repeatModeHandler(AdvancedRemote::RepeatMode mode){
  switch(mode){
  case AdvancedRemote::REPEAT_MODE_OFF:
    Serial.println("repeat off");
    playbackStatus &= ~REPEAT_MASK;
    break;
  case AdvancedRemote::REPEAT_MODE_ONE_SONG:
    Serial.println("repeat one");
    playbackStatus =  ((~REPEAT_MASK)&playbackStatus)| REPEAT_ONE;
    break;
  case AdvancedRemote::REPEAT_MODE_ALL_SONGS:
    Serial.println("repeat all");
    playbackStatus =  ((~REPEAT_MASK)&playbackStatus)| REPEAT_ALL;
    break;
  }
}
*/

void playlistPositionHandler(unsigned long playlistPosition){
  trackindex = playlistPosition;
}

void currentPlaylistSongCountHandler(unsigned long count){
  plistcount = count;
}

/*
void pollingHandler(AdvancedRemote::PollingCommand command, unsigned long playlistPositionOrelapsedTimeMs){
  switch(command){
  // track change. 'val' is the playlist position
  case AdvancedRemote::POLLING_TRACK_CHANGE: 
    trackindex = playlistPositionOrelapsedTimeMs;
    Serial.print("Track changed to index ");
    Serial.println(playlistPositionOrelapsedTimeMs, DEC);
    break;

  // elapsed time. 'val' is song playback time in ms
  case AdvancedRemote::POLLING_ELAPSED_TIME:
    playtime = playlistPositionOrelapsedTimeMs/1000;
    Serial.print("track elapsed time is ");
    Serial.println(playtime, DEC);
    break;

  default:
    break;
  }
}
*/

#ifdef DEBUG_MODE
/*
void iPodNameHandler(const char *ipodName){
    Serial.print("\tiPod name\t"); Serial.println(ipodName);  
}
*/
#endif


void setup(){
  ioinit();
  Serial.begin(9600); //12 Mbps, regardless of speed
  //setDebugPrint(Serial);
  //setLogPrint(Serial);
  nextUpdate=millis()+2000;
}

void loop(){
  switcher();
  ipodAvailable = true;
  
  //if(updateDisplay || (playbackStatus&PLAYING && activeInput == AUX_ACTIVE && nextUpdate<millis())){
  if(updateDisplay || (activeInput == AUX_ACTIVE && nextUpdate<millis())){
    digitalWrite(PIN_D6, ~digitalRead(PIN_D6));
    mBus.sendPlayingTrack(plistindex, trackindex, playtime, playbackStatus);
    mBus.send(0xE9401010A0A0002ull);
    updateDisplay = false;
    
    nextUpdate=millis()+500;
  }
  
#ifdef DEBUG_MODE
  #else if(nextUpdate<millis()){
  if(nextUpdate<millis()){
    Serial.print("\nDEBUG\n\tplayback status\t"); Serial.println(playbackStatus, BIN);
    Serial.print("\tinput\t"); 
    if(activeInput==IPOD_ACTIVE) Serial.println("iPod");
    else Serial.println("aux");
    Serial.print("\tipodAvailable\t"); Serial.println(ipodAvailable, BIN);
    
    Serial.print("\tplistindex\t"); Serial.println(plistindex);
    Serial.print("\ttrackindex\t"); Serial.println(trackindex);
    Serial.print("\tplaytime\t"); Serial.println(playtime);
    Serial.print("\tplistcount\t"); Serial.println(plistcount);
    Serial.print("\ttrackcount\t"); Serial.println(trackcount);

    //mBus.send(0xE9401010A0A0002ull);
    mBus.send(0xE930001AAAA0009ULL);
    nextUpdate=millis()+2000;
  }
#endif

  if(mBus.receive(&mbusMsg)){
    
    if(mbusMsg == 0x68){
      if(millis()-prevPing<1500){
        Serial.println("INFO: pings too fast, pausing.");
        playbackStatus = playbackStatus & (~PBSTATUS_MASK);
        playbackStatus |= PAUSED;
      }
      else{
        Serial.print("  HU: ping;  PBStatus: ");
        Serial.println(playbackStatus, HEX);
        delay(MBUS_DELAY);
        mBus.send(0xE8);
        delay(MBUS_DELAY);
        if(playbackStatus & PAUSED || (playbackStatus & PBSTATUS_MASK) == STOPPED){
          mBus.send(0x69);
          delay(MBUS_DELAY);
          //Serial.println("\t\t\tresend CD status");
          mBus.send(0xEF00000);
          delay(MBUS_DELAY);
          mBus.sendChangedCD(plistindex,trackindex);
          delay(MBUS_DELAY);
          mBus.sendCDStatus(plistindex);
          delay(MBUS_DELAY);
          if(activeInput==IPOD_ACTIVE)
            //mBus.sendPlayingTrack(plistindex, trackindex, playtime, playbackStatus);
            mBus.send(0xE930101AA000009ull);
          else
            mBus.send(0xE930601000A0009ULL);
        }
        else if(playbackStatus&PLAYING){
          mBus.send(0x78);
        }
        prevPing = millis();
      }
    }
    else if((uint64_t)(mbusMsg&MBUS_CMD_MASK) == MBUS_CMD){
      mBus.send(MBUS_DELAY);
      Serial.print("  HU: P/B state change -- ");
      switch(mbusMsg & 0xFFFull){
        case MBUS_CMD_PLAY:
          Serial.println("play");
          if(activeInput & IPOD_ACTIVE){
        
          }
          else{

            changePBStatus(PLAYING|REPEAT_ALL|SHUFFLE_SONGS);
          }  
          break;
        case MBUS_CMD_PAUSE:
          Serial.println("pause");
          if(activeInput & IPOD_ACTIVE){
          }
          else{
            //clear current state
            
            changePBStatus(PAUSED);
          }
          break;
        case MBUS_CMD_FFWD:
          Serial.println("ffwd");
          break;
        case MBUS_CMD_RWD:
          Serial.println("rew");
          break;
        default:
          Serial.println("??");
          break;
      }
      delay(MBUS_DELAY);
      mBus.sendPlayingTrack(plistindex, trackindex, playtime, playbackStatus);
      Serial.print("P/BS: ");
      Serial.println(playbackStatus, HEX);
    }
    else if((uint64_t)(mbusMsg&MBUS_SELCMD_MASK) == MBUS_SELCMD){
      Serial.print("  HU: select -- disc ");
      tmp = (mbusMsg & MBUS_SELCMD_DISCMASK)>>12;
      Serial.print(tmp, HEX);     
      
      tmp = (mbusMsg & MBUS_SELCMD_TRACKMASK)>>4;
      
      Serial.print(", track ");
      Serial.println(tmp, HEX);     
    }
    else if((uint64_t)(mbusMsg&MBUS_PBMODE_MASK) == MBUS_PBMODE){
      Serial.print("  HU: P/B mode change -- ");
      //switch(mbusMsg & ~MBUS_PBMODE_MASK){
        
        
        
      //}
    }
  }
}

void changePBStatus(uint8_t newstate){
  //clear any states that are being changed
  if(newstate & PBSTATUS_MASK)
    playbackStatus = playbackStatus & (~PBSTATUS_MASK);
  if(newstate & SHUFFLE_MASK)
    playbackStatus = playbackStatus & (PBSTATUS_MASK | REPEAT_MASK);
  if(newstate & REPEAT_MASK)
    playbackStatus = playbackStatus & (PBSTATUS_MASK | SHUFFLE_MASK);

  playbackStatus |= newstate;  
}
/* also useful iPod commands
getShuffleMode();
setShuffleMode(AdvancedRemote::SHUFFLE_MODE_ [OFF, SONGS, ALBUMS]  );

getRepeatMode();
setRepeatMode(AdvancedRemote::REPEAT_MODE_ [OFF, ONE_SONG, ALL_SONGS]);

switchToItem(AdvancedPlaylist::ITEM_ , ulong index);
//                                  PLAYLIST
//                                  ARTIST
//                                  ALBUM
//                                  GENRE
//                                  SONG
//                                  COMPOSER

switchToMainLibraryPlaylist();
executeSwitch(ulong index); //0xFFFFFFFF is always beginning of playlist

controlPlayback(AdvancedRemote::PLAYBACK_CONTROL_ );
//                                               PLAY_PAUSE
//                                               STOP
//                                               SKIP_FORWARD
//                                               SKIP_BACKWARD
//                                               FAST_FORWARD
//                                               FAST_REVERSE
//                                               STOP_FF_OR_REV
*/

//the code to switch inputs. handles toggling ipod play/pause, relay, lights, etc.
void switcher(){
  if(!digitalRead(TOGGLE_IPOD) && activeInput == AUX_ACTIVE && ipodAvailable){
    Serial.println("switching to iPod");
    //mbus.sendswitchingstatus(ipod); //send "AA00" or something
    mBus.send(0xE9300010A000009ULL);
    digitalWrite(LED_AUX,  0);
    digitalWrite(LED_IPOD, 1);
    
    //pause iPod if it's playing
    if(ipodAvailable && playbackStatus&PLAYING){
      //ipod.enable();
      //ipod.setPollingMode(AdvancedRemote::POLLING_START);
      //ipod.controlPlayback(AdvancedRemote::PLAYBACK_CONTROL_PLAY_PAUSE);
      playbackStatus = (playbackStatus & (~PBSTATUS_MASK)) | PLAYING;
    }
  }
  else if(!digitalRead(TOGGLE_AUX) && activeInput == IPOD_ACTIVE){
    Serial.println("switching to aux");
    //mbus.sendswitchingstatus(AUX);
    mBus.send(0xE930001AAAA0009ULL);
    digitalWrite(LED_AUX,  1);
    digitalWrite(LED_IPOD, 0);
#ifndef DISABLE_RELAY
#endif
    activeInput = AUX_ACTIVE;
    
  }
}


//set up pin directions and re-set the relay to auxin
void ioinit(){
  pinMode(LED_AUX, OUTPUT); // aux led
  pinMode(LED_IPOD, OUTPUT); // ipod led
  digitalWrite(LED_AUX,    1);
  digitalWrite(LED_IPOD,   0);
  
  pinMode(TOGGLE_AUX,  INPUT_PULLUP); // toggle, aux
  pinMode(TOGGLE_IPOD, INPUT_PULLUP); // toggle, ipod

  //set the input to the aux by default until an iPod is detected
  activeInput = AUX_ACTIVE;
}
