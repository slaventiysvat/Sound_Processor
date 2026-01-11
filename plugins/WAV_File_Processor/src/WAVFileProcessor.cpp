#include "wavp/WAVFileProcessor.hpp"
#include <iostream>
#include <string>
#include "AudioFile.h" // external/AudioFileWAVReader/AudioFile.h
using namespace wavp;
extern "C" {
SP_API void wavfp_print_summary(const char* path){
  try{ AudioFile<float> f; if(!f.load(std::string(path))){ std::cerr<<"Failed to load: "<<path<<"\n"; return; } f.printSummary(); }
  catch(...){ std::cerr<<"Exception in wavfp_print_summary\n"; }
}
SP_API int wavfp_copy_to(const char* inPath, const char* outPath){
  try{ AudioFile<float> f; if(!f.load(std::string(inPath))) return 0; return f.save(std::string(outPath), AudioFileFormat::Wave)?1:0; }
  catch(...){ return 0; }
}
SP_API sp::ISignalProcessor* sp_create(){ return new WAVFileProcessor(); }
SP_API void sp_destroy(sp::ISignalProcessor* p){ delete p; }
SP_API const char* sp_name(){ return "WAV_File_Processor"; }
SP_API int sp_version_major(){ return 1; }
SP_API int sp_version_minor(){ return 0; }
}
