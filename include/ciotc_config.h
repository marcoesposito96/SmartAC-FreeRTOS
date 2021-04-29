// const char *ssid = "dd-wrt";
// const char *password = "kindergransorpresa";

const char *ssid = "FASTWEB-C16F97";
const char *password = "alberogiallo12";

// Cloud IoT details.
const char *project_id = "smart-ac-embedded";
const char *location = "europe-west1";
const char *registry_id = "iot-reg";

#define fabio

#ifdef fabio
const char *device_id = "esp32";
#endif

#ifdef marco
const char *device_id = "esp32-2";
#endif


// Configuration for NTP
const char* ntp_primary = "pool.ntp.org";
const char* ntp_secondary = "time.nist.gov";

// To get the private key run (where private-key.pem is the ec private key
// used to create the certificate uploaded to google cloud iot):
// openssl ec -in <private-key.pem> -noout -text
// and copy priv: part.
// The key length should be exactly the same as the key length bellow (32 pairs
// of hex digits). If it's bigger and it starts with "00:" delete the "00:". If
// it's smaller add "00:" to the start. If it's too big or too small something
// is probably wrong with your key.
const char* private_key_str =
    "dd:6a:c2:7e:b8:26:44:93:c0:7b:c5:d8:7a:11:a6:"
    "79:e0:1b:40:8d:9f:1d:2e:98:7f:0d:4f:40:9a:66:"
    "a7:cd";

// Time (seconds) to expire token += 20 minutes for drift
const int jwt_exp_secs = 1200; // Maximum 24H (3600*24)

// In case we ever need extra topics
const int ex_num_topics = 0;
const char* ex_topics[ex_num_topics];
//const int ex_num_topics = 1;
//const char* ex_topics[ex_num_topics] = {
//  "/devices/my-device/tbd/#"
//};

const char *root_cert =
"-----BEGIN CERTIFICATE-----\n"
"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\n"
"bdCMtVYgQ/jQ/NSiHmU00D/zNPUYrOV/fx1I\n"
"-----END CERTIFICATE-----";
