#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

typedef pair<vector<double>, int> pFrequencyDuration; // (hertz , seconds);

// HEADER
const string chunkID = "RIFF";
const string chunkSize = "----"; // 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
const string format = "WAVE";

// FORMAT SUBCHUNK
const string subChunk1Id = "fmt ";
const int subChunk1Size = 16;
const int audioFormat = 1;
const int numChannels = 1;
const int sampleRate = 44100;
const int byteRate = sampleRate * numChannels * 16/8; // 16 = bitsPerSample
const int blockAlign = numChannels * 16/8; // 16 = bitsPerSample
const int bitsPerSample = 16;

// DATA SUBCHUNK
const string subChunk2ID = "data";
const string subChunk2Size = "----"; // numSamples * numChannels * bitsPerSample/8

void toByte(ofstream &file, int value, int size){
    file.write(reinterpret_cast<const char*>(&value), size);
}

// writing the basic structure of the WAV file
void writeChunks(ofstream &file){
        file << chunkID;
        file << chunkSize;
        file << format;

        file << subChunk1Id;
        toByte(file, subChunk1Size, 4);
        toByte(file, audioFormat, 2);
        toByte(file, numChannels, 2);
        toByte(file, sampleRate, 4);
        toByte(file, byteRate, 4);
        toByte(file, blockAlign, 2);
        toByte(file, bitsPerSample, 2);

        file << subChunk2ID;
        file << subChunk2Size;
}

const int maxAmplitude = 32760; // aprox max value for 16bits

void writeData(ofstream &file, const vector<double> frequency, const int duration){
    double amplitude;
    double value;
    double channel;

    int numberOfNotes = frequency.size();

    for (int i = 0 ; i < sampleRate * duration; i++){
        amplitude = maxAmplitude * (1.0 - (double)i / (sampleRate * duration )); // fade the note
        double time = (double)i/sampleRate;
        value = 0.0;
        for (int j = 0; j < numberOfNotes; j++) {
            value += sin(2.0 * 3.14 * frequency[j] * time);
        }
        value = value / numberOfNotes;
        channel = amplitude * value;
        toByte(file, channel, 2);
    }
}

void twinleTwinkle(ofstream &file){
    vector<pFrequencyDuration> notes;

    // twinkle twinkle little star
    notes.push_back({{261.63} , 1});
    notes.push_back({{261.63} , 1});
    notes.push_back({{392.00} , 1});
    notes.push_back({{392.00} , 1});
    notes.push_back({{440.00} , 1});
    notes.push_back({{440.00} , 1});
    notes.push_back({{392.00} , 2});

    for (int i = 0; i < notes.size(); i++){
        writeData(file, notes[i].first, notes[i].second);
    }
}

void chords(ofstream &file){
    vector<pFrequencyDuration> notes;

    // D, G, E
    notes.push_back({{293.66, 369.99, 440.00} , 1});
    notes.push_back({{392.00, 493.88, 587.33} , 1});
    notes.push_back({{329.63, 415.30, 493.88} , 1});

    for (int i = 0; i < notes.size(); i++){
        writeData(file, notes[i].first, notes[i].second);
    }
}

int main(){
    ofstream wav;
    wav.open("twinleTwinkle.wav", ios::binary);

    if(wav.is_open()){
        writeChunks(wav);

        int start = wav.tellp();        // audio start

        twinleTwinkle(wav);

        int end = wav.tellp();          // eof

        wav.seekp(start - 4);           // going back to subChunk2Size
        toByte(wav, end - start, 4);    // writing the value

        wav.seekp(4, ios::beg);         // going back to chunkSize
        toByte(wav, end - 8, 4);        // writing the value
    }

    wav.close();

    wav.open("chords.wav", ios::binary);
    if(wav.is_open()){
        writeChunks(wav);

        int start = wav.tellp();        // audio start

        chords(wav);

        int end = wav.tellp();          // eof

        wav.seekp(start - 4);           // going back to subChunk2Size
        toByte(wav, end - start, 4);    // writing the value

        wav.seekp(4, ios::beg);         // going back to chunkSize
        toByte(wav, end - 8, 4);        // writing the value
    }
    wav.close();

}