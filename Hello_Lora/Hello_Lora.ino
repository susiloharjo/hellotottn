

// MIT License
// https://github.com/gonzalocasas/arduino-uno-dragino-lorawan/blob/master/LICENSE
// Based on examples from https://github.com/matthijskooijman/arduino-lmic
// Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman

// Adaptions: Andreas Spiess

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
//#include <credentials.h>
//
//#ifdef CREDENTIALS
//static const u1_t NWKSKEY[16] = NWKSKEY1;
//static const u1_t APPSKEY[16] = APPSKEY1;
//static const u4_t DEVADDR = DEVADDR1;
//#else
//static const u1_t NWKSKEY[16] = { 0x29, 0x06, 0x32, 0x35, 0x4E, 0xF7, 0xE2, 0x1F, 0xF5, 0x00, 0xFA, 0x07, 0xF8, 0x67, 0x2B, 0x05 };
//static const u1_t APPSKEY[16] = { 0x9B, 0x42, 0x95, 0x16, 0x52, 0x67, 0x15, 0xD5, 0xA0, 0x74, 0x49, 0x6F, 0x5C, 0xB1, 0x69, 0xD1 };
//static const u4_t DEVADDR = 0x260412BD;
//#endif

static const PROGMEM u1_t NWKSKEY[16] = { 0xCD, 0xD2, 0x30, 0x0F, 0x05, 0xA3, 0x61, 0x31, 0x01, 0xC9, 0xA3, 0x1B, 0x1E, 0xDD, 0x73, 0xFD };
static const PROGMEM u1_t APPSKEY[16] = { 0xEB, 0xE2, 0x36, 0x1B, 0xDC, 0x5A, 0x4E, 0x68, 0x54, 0x3C, 0xA5, 0xA2, 0x88, 0x13, 0x78, 0xE7 };
static const u4_t DEVADDR = 0x26041BFA;

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 30;

// Pin mapping Dragino Shield
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};
void onEvent (ev_t ev) {
    if (ev == EV_TXCOMPLETE) {
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        // Schedule next transmission
        os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
    }
}

void do_send(osjob_t* j){
    // Payload to send (uplink)
    static uint8_t message[] = "hi";

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, message, sizeof(message)-1, 0);
        Serial.println(F("Sending uplink packet..."));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("Starting..."));

    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters.
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);

    LMIC_setupChannel(1, 868800000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    
    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}

