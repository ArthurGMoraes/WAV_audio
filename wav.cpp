#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

//----------------------------------------------------------------------------------------------//
//                                      code setup                                              //
//----------------------------------------------------------------------------------------------//
typedef pair<vector<double>, double> pFrequencyDuration; // (hertz , seconds);

struct ADSR {
    double attackTime;   // in seconds
    double decayTime;    // in seconds
    double sustainLevel; // between 0.0 and 1.0
    double releaseTime;  // in seconds
};

ADSR guitarEnvelope = {
    0.01,  
    5.0,  
    0.0,    
    4   
};

const int maxAmplitude = 32760; // aprox max value for 16bits

double makeSound(const int, const pFrequencyDuration, ADSR);

//----------------------------------------------------------------------------------------------//
//                                      .WAV structure                                          //
//----------------------------------------------------------------------------------------------//                                    
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

//----------------------------------------------------------------------------------------------//
//                                         writng data                                          //
//----------------------------------------------------------------------------------------------//
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

void writeData(ofstream &file, const pFrequencyDuration notes, ADSR envelope){
    double channel;

    for (int i = 0 ; i < sampleRate * notes.second; i++){
        channel = makeSound(i, notes, envelope);
        toByte(file, channel, 2);
    }
}

//----------------------------------------------------------------------------------------------//
//                                       creating sounds                                        //
//----------------------------------------------------------------------------------------------//

double applyHardClipping(double sample) {
    const double threshold = 0.08; //  0 < threshold  <= 1

    // Soft clipping
    if (sample >= 0) {
        sample = fmin(sample, threshold);
    } else {
        sample = fmax(sample, -threshold);
    }

    return sample /= threshold;
}

double applyGain(double sample){
    double gain = 6.00;
    sample *= gain;
    return sample;
}

double applyADSR(double time, double noteDuration, ADSR envelope) {
    double amplitude = 0.0;

    if (time < envelope.attackTime) {
        // Attack phase
        amplitude = (time / envelope.attackTime);
    } else if (time < envelope.attackTime + envelope.decayTime) {
        // Decay phase
        double decayProgress = (time - envelope.attackTime) / envelope.decayTime;
        amplitude = 1.0 - (1.0 - envelope.sustainLevel) * decayProgress;
    } else if (time < noteDuration) {
        // Sustain phase
        amplitude = envelope.sustainLevel;
    } else if (time < noteDuration + envelope.releaseTime) {
        // Release phase
        double releaseProgress = (time - noteDuration) / envelope.releaseTime;
        amplitude = envelope.sustainLevel * (1.0 - releaseProgress);
    } else {
        // After release
        amplitude = 0.0;
    }

    return amplitude;
}

double makeSound(const int i, const pFrequencyDuration notes, ADSR envelope){
    double amplitude, value, fundamental, harmonic1, harmonic2, delayedTime;
    double duration = notes.second;
    vector<double> frequency = notes.first;
    int numberOfNotes = notes.first.size();
    vector<double> startTimes(numberOfNotes);
    for (int j = 0; j < numberOfNotes; j++) {
        startTimes[j] = j * 0.05; // Each note starts later by strumDelay
    }
    
    double time = (double)i/sampleRate;
    amplitude = applyADSR(time, duration, envelope) * (maxAmplitude); // fade the note
    
    value = 0.0;
    for (int i = 0; i < numberOfNotes; i++) {
        if (time >= startTimes[i]) {
            delayedTime = time - startTimes[i];
            fundamental = sin(2.0 * 3.14 * frequency[i] * delayedTime);
            harmonic1 = 0.5 * sin(2.0 * 3.14 * 2 * frequency[i] * delayedTime);
            harmonic2 = 0.25 * sin(2.0 * 3.14 * 3 * frequency[i] * delayedTime);
            value += fundamental + harmonic1 + harmonic2;
        }
        
    }
    value = value / (numberOfNotes*3);
    value = applyGain(value);
    value = applyHardClipping(value);
    value = amplitude * value;
    return value;
}

void twinleTwinkle(ofstream &file, ADSR envelope){
    vector<pFrequencyDuration> notes;

    // twinkle twinkle little star
    notes.push_back({{261.63} , 1.0});
    notes.push_back({{261.63} , 1.0});
    notes.push_back({{392.00} , 1.0});
    notes.push_back({{392.00} , 1.0});
    notes.push_back({{440.00} , 1.0});
    notes.push_back({{440.00} , 1.0});
    notes.push_back({{392.00} , 2.0});

    for (int i = 0; i < notes.size(); i++){
        writeData(file, notes[i], envelope);
    }
}

void chords(ofstream &file, ADSR envelope){
    vector<pFrequencyDuration> notes;

    // D, G, E
    notes.push_back({{147.00, 220.00, 294.00 , 370.00} , 6.0});
    notes.push_back({{98.00, 123.00, 147.00, 196.00, 294.00, 392.00} , 3.0});
    notes.push_back({{82.00, 123.00 , 165.00, 208.00} , 3.0});

    for (int i = 0; i < notes.size(); i++){
        writeData(file, notes[i], envelope);
    }
}

//----------------------------------------------------------------------------------------------//

int main(){
    ofstream wav;
    wav.open("twinleTwinkle.wav", ios::binary);

    if(wav.is_open()){
        writeChunks(wav);

        int start = wav.tellp();        // audio start

        twinleTwinkle(wav, guitarEnvelope);

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

        chords(wav, guitarEnvelope);

        int end = wav.tellp();          // eof

        wav.seekp(start - 4);           // going back to subChunk2Size
        toByte(wav, end - start, 4);    // writing the value

        wav.seekp(4, ios::beg);         // going back to chunkSize
        toByte(wav, end - 8, 4);        // writing the value
    }
    wav.close();

}