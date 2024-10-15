#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

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

const int maxAmplitude = 32760; // aprox max value for 16bits
void writeData(ofstream &file, const int duration){
    double amplitude;
    double value;
    double channel;

    // twinkle twinkle little star
    for (int i = 0 ; i < sampleRate * (duration / 8); i++){
        amplitude = maxAmplitude * (1.0 - (double)i / (sampleRate * (duration / 8))); // fade the note
        double time = (double)i/sampleRate;
        value = sin(2 * 3.14 * 261.63 * time);
        channel = amplitude * value;
        toByte(file, channel, 2);
    }
    for (int i = 0 ; i < sampleRate * (duration / 8); i++){
        amplitude = maxAmplitude * (1.0 - (double)i / (sampleRate * (duration / 8)));
        double time = (double)i/sampleRate;
        value = sin(2 * 3.14 * 261.63 * time);
        channel = amplitude * value;
        toByte(file, channel, 2);
    }
    for (int i = 0 ; i < sampleRate * (duration / 8); i++){
        amplitude = maxAmplitude * (1.0 - (double)i / (sampleRate * (duration / 8)));
        double time = (double)i/sampleRate;
        value = sin(2 * 3.14 * 392.00 * time);
        channel = amplitude * value;
        toByte(file, channel, 2);
    }
    for (int i = 0 ; i < sampleRate * (duration / 8); i++){
        amplitude = maxAmplitude * (1.0 - (double)i / (sampleRate * (duration / 8)));
        double time = (double)i/sampleRate;
        value = sin(2 * 3.14 * 392.00 * time);
        channel = amplitude * value;
        toByte(file, channel, 2);
    }
    for (int i = 0 ; i < sampleRate * (duration / 8); i++){
        amplitude = maxAmplitude * (1.0 - (double)i / (sampleRate * (duration / 8)));
        double time = (double)i/sampleRate;
        value = sin(2 * 3.14 * 440.00 * time);
        channel = amplitude * value;
        toByte(file, channel, 2);
    }
    for (int i = 0 ; i < sampleRate * (duration / 8); i++){
        amplitude = maxAmplitude * (1.0 - (double)i / (sampleRate * (duration / 8)));
        double time = (double)i/sampleRate;
        value = sin(2 * 3.14 * 440.00 * time);
        channel = amplitude * value;
        toByte(file, channel, 2);
    }
    for (int i = 0 ; i < sampleRate * (duration / 4); i++){
        amplitude = maxAmplitude * (1.0 - (double)i / (sampleRate * (duration / 4)));
        double time = (double)i/sampleRate;
        value = sin(2 * 3.14 * 392.00 * time);
        channel = amplitude * value;
        toByte(file, channel, 2);
    }
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

int main(){
    ofstream wav;
    wav.open("sound.wav", ios::binary);

    if(wav.is_open()){
        writeChunks(wav);

        int start = wav.tellp();        // audio start

        writeData(wav, 8);

        int end = wav.tellp();          // eof

        wav.seekp(start - 4);           // going back to subChunk2Size
        toByte(wav, end - start, 4);    // writing the value

        wav.seekp(4, ios::beg);         // going back to chunkSize
        toByte(wav, end - 8, 4);        // writing the value
    }

    wav.close();
}