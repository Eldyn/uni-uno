#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @file base64.hpp
 * @brief Libreria standalone per codifica e decodifica efficiente in formato Base64.
 * * Fornisce funzioni inline per convertire array di byte arbitrari in stringhe
 * ASCII Base64 e viceversa, utilizzando manipolazione diretta dei bit (bit shifting)
 * per garantire alte prestazioni.
 */

namespace Base64 {
    /**
     * @brief Alfabeto standard utilizzato per la codifica Base64.
     * Contiene i 64 caratteri (A-Z, a-z, 0-9, +, /) indicizzati da 0 a 63.
     * @tag CMN-B64-CONST-001
     */
    inline constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    /**
     * @brief Valore sentinella che indica un carattere non valido.
     * Utilizzato all'interno della tabella di decodifica per marcare i byte ASCII
     * che non fanno parte dell'alfabeto Base64.
     * @tag CMN-B64-CONST-002
     */
    inline constexpr uint8_t kInvalid = 0xFF;

    /**
     * @brief Tabella di lookup precalcolata a compile-time per la decodifica.
     * * Mappa il valore ASCII di un carattere (0-255) al suo corrispondente valore intero 
     * a 6-bit (0-63). I caratteri non appartenenti all'alfabeto sono mappati a `kInvalid`.
     * L'utilizzo di una lambda `constexpr` permette di generare la tabella a tempo di 
     * compilazione, eliminando l'overhead a runtime.
     * @tag CMN-B64-CONST-003
     */
    inline constexpr std::array<uint8_t, 256> kDecodeTable = []() {
        std::array<uint8_t, 256> table{};
        table.fill(kInvalid);

        for (uint8_t i = 0; i < 64; ++i) {
            table[static_cast<uint8_t>(kAlphabet[i])] = i;
        }

        return table;
    }();

    /**
     * @brief Converte dati binari grezzi (bytes) nella loro rappresentazione Base64.
     * * Prende dati binari arbitrari e restituisce la stringa ASCII codificata.
     * Ogni 3 byte di input vengono convertiti in 4 caratteri di output. Il padding 
     * finale con il carattere '=' viene aggiunto automaticamente per compensare le code.
     * * Aritmetica dei bit:
     * Dati i byte [A, B, C] impacchettati in un intero a 24-bit:
     * - bit 23-18 → indice 0
     * - bit 17-12 → indice 1
     * - bit 11- 6 → indice 2
     * - bit  5- 0 → indice 3
     * * @param data Vettore contenente i byte raw da codificare.
     * @return std::string Stringa ASCII codificata in Base64.
     * @tag CMN-B64-MTH-001
     */
    inline std::string Encode(const std::vector<uint8_t>& data) {
        std::string out;
        out.reserve(((data.size() + 2) / 3) * 4);

        for (size_t i = 0; i < data.size(); i += 3) {
            uint32_t b = static_cast<uint32_t>(data[i]) << 16;
            if (i + 1 < data.size()) b |= static_cast<uint32_t>(data[i + 1]) << 8;
            if (i + 2 < data.size()) b |= static_cast<uint32_t>(data[i + 2]);

            out += kAlphabet[(b >> 18) & 0x3F];
            out += kAlphabet[(b >> 12) & 0x3F];

            out += (i + 1 < data.size()) ? kAlphabet[(b >> 6) & 0x3F] : '=';
            out += (i + 2 < data.size()) ? kAlphabet[ b       & 0x3F] : '=';
        }
        return out;
    }

    /**
     * @brief Operazione inversa di Encode, ripristina la stringa Base64 in byte.
     * * Esegue una validazione di ogni carattere stringa tramite `kDecodeTable` 
     * ed estrae i 6 bit originari.
     * * @param encoded Stringa codificata in Base64 (può includere il padding '=').
     * @return std::vector<uint8_t> I byte raw originali decodificati.
     * @throws std::invalid_argument Se `encoded` contiene caratteri illegali 
     * o ha una lunghezza che non è un multiplo di 4.
     * @tag CMN-B64-MTH-002
     */
    inline std::vector<uint8_t> Decode(const std::string& encoded) {
        if (encoded.size() % 4 != 0) {
            throw std::invalid_argument("base64::Decode: length must be a multiple of 4");
        }

        std::vector<uint8_t> out;
        out.reserve((encoded.size() / 4) * 3);

        for (size_t i = 0; i < encoded.size(); i += 4) {
            auto idx = [&](size_t pos) -> uint32_t {
                uint8_t c = static_cast<uint8_t>(encoded[pos]);
                uint8_t v = kDecodeTable[c];
                if (v == kInvalid && encoded[pos] != '=') {
                    throw std::invalid_argument(
                        std::string("base64::Decode: invalid character '") + encoded[pos] + "'");
                }
                return (encoded[pos] == '=') ? 0u : static_cast<uint32_t>(v);
            };

            // Rimpacchetta quattro valori a 6-bit in un intero a 24-bit.
            uint32_t b = (idx(i) << 18) | (idx(i+1) << 12) | (idx(i+2) << 6) | idx(i+3);

            out.push_back(static_cast<uint8_t>((b >> 16) & 0xFF));
            if (encoded[i + 2] != '=') out.push_back(static_cast<uint8_t>((b >> 8) & 0xFF));
            if (encoded[i + 3] != '=') out.push_back(static_cast<uint8_t>( b       & 0xFF));
        }
        return out;
    }

}
