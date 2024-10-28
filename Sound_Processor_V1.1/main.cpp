#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h >   //for memcmp
#include <conio.h>
#include <time.h>
#include "defines_values.h"
#include "special_math.h"
#include "special_math_new.h"
#include "s_malloc.h"
#include "AudioFile.h"
#include <Eigen/Dense>
#include <Eigen/Dense>
#include <iostream>


using Eigen::Matrix3d;
using Eigen::Vector3d;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::VectorXi;
//test

//allocating memory for all arrays
short in_m[NUM_OF_SAMPLES] = { 0 }; //16bit values
short in_n[NUM_OF_SAMPLES] = { 0 }; //16bit values
short out[NUM_OF_SAMPLES] = { 0 };  //16bit values

void LoadAndSaveAudioFileAndPrintSummary();

int main(int argc, char* argv[]) {

	LoadAndSaveAudioFileAndPrintSummary();
	_getch();
	return 0;

}

//=======================================================================
void LoadAndSaveAudioFileAndPrintSummary()
{
    //---------------------------------------------------------------
    std::cout << "**********************" << std::endl;
    std::cout << "Load Audio WAV File and Print Summary" << std::endl;
    std::cout << "**********************" << std::endl << std::endl;

    //---------------------------------------------------------------
    // 1. Set a file path to an audio file on your machine
    const std::string filePathMain = std::string("../raw_in/alchimia_1_main_mic16_16.wav");
	const std::string filePathNoise = std::string("../raw_in/alchimia_1_main_mic16_16.wav");
    //---------------------------------------------------------------
    // 2. Create an AudioFile object and load the audio file

    AudioFile<uint16_t> a;
    bool loadedOKMain = a.load(filePathMain); 
	/** If you hit this assert then the file path above
     probably doesn't refer to a valid audio file */
	assert(loadedOKMain);
	AudioFile<uint16_t> b;
	bool loadedOKNoise = b.load(filePathNoise);
    assert(loadedOKNoise);

    //---------------------------------------------------------------
    // 3. Let's print out some key details
	std::cout << " =============== Main Signal ================ " << std::endl;
    std::cout << "Bit Depth: " << a.getBitDepth() << std::endl;
    std::cout << "Sample Rate: " << a.getSampleRate() << std::endl;
    std::cout << "Num Channels: " << a.getNumChannels() << std::endl;
    std::cout << "Length in Seconds: " << a.getLengthInSeconds() << std::endl;
    std::cout << std::endl;

	std::cout << " =============== Noise Signal ================ " << std::endl;
	std::cout << "Bit Depth: " << b.getBitDepth() << std::endl;
	std::cout << "Sample Rate: " << b.getSampleRate() << std::endl;
	std::cout << "Num Channels: " << b.getNumChannels() << std::endl;
	std::cout << "Length in Seconds: " << b.getLengthInSeconds() << std::endl;
	std::cout << std::endl;

	std::cout << "**********************" << std::endl;
	std::cout << "Running Example: Write WAV To Audio File" << std::endl;
	std::cout << "**********************" << std::endl << std::endl;


	//---------------------------------------------------------------
	// 4. Save the AudioFile

	std::string filePath = "sine-wave.wav"; // change this to somewhere useful for you
	bool SuccsessM = a.save("../raw_out/main-wave.wav", AudioFileFormat::Wave);
	bool SuccsessN = b.save("../raw_out/noise-wave.wav", AudioFileFormat::Wave);

	if (SuccsessM && SuccsessN) {

		std::cout << "**********************" << std::endl;
		std::cout << "Write WAV To Audio File Successfully" << std::endl;
		std::cout << "**********************" << std::endl << std::endl;
	
	}
	else {
	
		std::cout << "**********************" << std::endl;
		std::cout << "Huston, we have a problem" << std::endl;
		std::cout << "**********************" << std::endl << std::endl;
	
	}
}

