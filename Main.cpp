#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <limits>

using namespace std;

struct Color {
    int r, g, b;

    // Operator porównywania kolorów, potrzebny do używania Color jako klucza w mapie
    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
};

// Funkcja haszująca dla struktury Color
namespace std {
    template<>
    struct hash<Color> {
        size_t operator()(const Color& color) const {
            return hash<int>()(color.r) ^ hash<int>()(color.g) << 1 ^ hash<int>()(color.b) << 2;
        }
    };
}

// Funkcja do wczytywania obrazów w formatach PPM, PGM i PBM
bool loadImage(const string& filename, vector<int>& pixels, vector<Color>& colors, int& width, int& height, int& maxVal) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cout << "Nie można otworzyć pliku." << endl;
        return false;
    }

    string header;
    file >> header;
    file.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignoruje resztę linii po nagłówku

    // Pomijanie komentarzy
    char ch;
    file.get(ch);
    while (ch == '#') {
        file.ignore(numeric_limits<streamsize>::max(), '\n');
        file.get(ch);
    }
    file.unget();

    // Wczytanie szerokości, wysokości i ewentualnie maksymalnej wartości koloru
    file >> width >> height;
    if (header == "P2" || header == "P3" || header == "P5" || header == "P6") {
        file >> maxVal;
    }
    file.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignoruje resztę linii

    if (header == "P3" || header == "P6") {
        colors.resize(width * height);
        unsigned char rgb[3];
        for (int i = 0; i < width * height; ++i) {
            file.read(reinterpret_cast<char*>(rgb), 3);
            colors[i] = { rgb[0], rgb[1], rgb[2] };
        }
        return true;
    }
    else if (header == "P2" || header == "P5") {
        pixels.resize(width * height);
        unsigned char value;
        for (int i = 0; i < width * height; ++i) {
            file.read(reinterpret_cast<char*>(&value), 1);
            pixels[i] = value;
        }
        return true;
    }
    else if (header == "P1" || header == "P4") {
        pixels.resize(width * height);
        int rowSize = (width + 7) / 8; // Ilość bajtów na wiersz
        vector<unsigned char> row(rowSize);
        for (int y = 0; y < height; y++) {
            file.read(reinterpret_cast<char*>(&row[0]), rowSize);
            for (int x = 0; x < width; x++) {
                int byteIndex = x / 8;
                int bitIndex = 7 - (x % 8);
                pixels[y * width + x] = (row[byteIndex] >> bitIndex) & 1;
            }
        }
        return true;
    }

    cout << "Nieobsługiwany format pliku." << endl;
    return false;
}

// Funkcja do analizy danych pikselowych na podstawie formatu
void analyzeImage(const vector<int>& pixels, const vector<Color>& colors, int width, int height, const string& format) {
    if (format == "P3" || format == "P6") { // PPM
        unordered_map<Color, int> colorCount;
        for (const auto& color : colors) {
            ++colorCount[color];
        }
        Color mostFrequent;
        int maxCount = 0;
        for (const auto& pair : colorCount) {
            if (pair.second > maxCount) {
                maxCount = pair.second;
                mostFrequent = pair.first;
            }
        }
        cout << "Najczęściej występujący kolor to "
            << mostFrequent.r << "-" << mostFrequent.g << "-" << mostFrequent.b
            << " i wystąpił " << maxCount << " razy" << endl;
        cout << "Liczba unikalnych kolorów: " << colorCount.size() << endl;
    }
    else if (format == "P1" || format == "P2" || format == "P5" || format == "P4") { // PGM or PBM
        unordered_map<int, int> grayscaleCount;
        for (int pixel : pixels) {
            ++grayscaleCount[pixel];
        }
        int mostFrequentGray = 0, maxCount = 0;
        for (const auto& pair : grayscaleCount) {
            if (pair.second > maxCount) {
                maxCount = pair.second;
                mostFrequentGray = pair.first;
            }
        }
        cout << "Najczęściej występujący odcień szarości: " << mostFrequentGray
            << " i wystąpił " << maxCount << " razy" << endl;
        cout << "Liczba unikalnych odcieni szarości: " << grayscaleCount.size() << endl;
    }
}

int main() {
    setlocale(LC_CTYPE, "Polish");

    string filename;
    string format;
    do {
        cout << "Podaj nazwę pliku: ";
        cin >> filename;

        vector<int> pixels;
        vector<Color> colors;
        int width, height, maxVal = 255;
        if (loadImage(filename, pixels, colors, width, height, maxVal)) {
            // Przypisanie formatu na podstawie rodzaju danych
            format = (colors.size() > 0) ? "P6" : (pixels.size() && maxVal > 1) ? "P5" : (pixels.size() && maxVal == 1) ? "P4" : "P1";
            cout << "Szerokość obrazu: " << width << endl;
            cout << "Wysokość obrazu: " << height << endl;
            analyzeImage(pixels, colors, width, height, format);
        }

        cout << "Czy chcesz wczytać kolejny plik (tak/nie): ";
        cin >> filename;
    } while (filename == "tak");

    return 0;
}
