#pragma once

namespace eloq {
    namespace rtls {
        class FeaturesConverter {
        public:
            float features[40];

            /**
            * Constructor
            */
            FeaturesConverter(Eloquent::RTLS::Scanner& scanner, Classifier& classifier) :
                _scanner(&scanner),
                _classifier(&classifier),
                _verbose(false) {
                memset(features, 0, 40);
            }

            /**
            * Toggle verbose output
            * @param enabled
            */
            void verbose(bool enabled = true) {
                _verbose = enabled;
            }

            /**
            * Perform scan and convert to features
            */
            String predict() {
                memset(features, 0, 40);
                _scanner->scan();

                /* build features */
                while (_scanner->hasNext()) {
                    _scanner->next();

                    if (!_scanner->isValid())
                        continue;

                    String identifier = _scanner->identifier();
                    const int16_t rssi = _scanner->rssi();

                    if (identifier == "#NET-CLARO-WIFI") features[0] = rssi;
                    if (identifier == "11B") features[1] = rssi;
                    if (identifier == "2.4G Net Virtua 42") features[2] = rssi;
                    if (identifier == "ANDFS-2G") features[3] = rssi;
                    if (identifier == "Animus-2.4G") features[4] = rssi;
                    if (identifier == "Azulao") features[5] = rssi;
                    if (identifier == "BATISTA 2.4") features[6] = rssi;
                    if (identifier == "BRABOS_HOUSE_2G") features[7] = rssi;
                    if (identifier == "Bernardi-2G") features[8] = rssi;
                    if (identifier == "CAMARGO") features[9] = rssi;
                    if (identifier == "CAVALINI") features[10] = rssi;
                    if (identifier == "CLARO_2G090BF0") features[11] = rssi;
                    if (identifier == "CLARO_2G27EB92") features[12] = rssi;
                    if (identifier == "CLARO_2G9642D0") features[13] = rssi;
                    if (identifier == "Caio_2G") features[14] = rssi;
                    if (identifier == "Conectando") features[15] = rssi;
                    if (identifier == "ERIKA.2G") features[16] = rssi;
                    if (identifier == "Familia Ab-2.4G") features[17] = rssi;
                    if (identifier == "GOMES") features[18] = rssi;
                    if (identifier == "GT casa") features[19] = rssi;
                    if (identifier == "Giovanna") features[20] = rssi;
                    if (identifier == "Giro73b") features[21] = rssi;
                    if (identifier == "Guto Rapido") features[22] = rssi;
                    if (identifier == "HERNANDES") features[23] = rssi;
                    if (identifier == "IndgriMarce") features[24] = rssi;
                    if (identifier == "JORGES") features[25] = rssi;
                    if (identifier == "LIMEIRA1015") features[26] = rssi;
                    if (identifier == "LUIZ CARLOS") features[27] = rssi;
                    if (identifier == "MANUANA") features[28] = rssi;
                    if (identifier == "MARTINS_2.4G") features[29] = rssi;
                    if (identifier == "Maju2.4") features[30] = rssi;
                    if (identifier == "Marcos_2G") features[31] = rssi;
                    if (identifier == "Medrado2g") features[32] = rssi;
                    if (identifier == "NET_2GE99BEE") features[33] = rssi;
                    if (identifier == "Neverland-2.4G") features[34] = rssi;
                    if (identifier == "Stefani") features[35] = rssi;
                    if (identifier == "TAMPINHA_2.4G") features[36] = rssi;
                    if (identifier == "VeraKokas") features[37] = rssi;
                    if (identifier == "gerosa") features[38] = rssi;
                    if (identifier == "ronaldoterapin") features[39] = rssi;
                    
                }
                /**
                dump();

                /** predict **/
                int prediction = _classifier->predict(features);

                switch (prediction) {
                     case 0: return "setor C - quadrante 2";
                     case 1: return "setor C - quadrante 1";
                    
                }

                return "ERROR";
            }

            /**
             * Print features vector
             
            void dump() {
                if (!_verbose)
                    return;

                Serial.print("Features: [");
                Serial.print(features[0]);

                for (int i = 1; i < 40; i++) {
                    Serial.print(", ");
                    Serial.print(features[i]);
                }

                Serial.println("]");
            }
            */

        protected:
            bool _verbose;
            Eloquent::RTLS::Scanner *_scanner;
            Classifier *_classifier;
        };
    }
}
