/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"

#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Detector 

Usage:
    get_pitch [--umbral=U] [--window=W] [--clipping=C] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    --umbral=U  Valor del umbral para discernir entre una trama sonora y silencio [default: 0.91]
    --window=W  Type of window. Available types: Rectangular, Hamming [default: RECT]
    --clipping=C  Valor del umbral para discernir entre una trama sonora y silencio [default: 0.001]  
    -h, --help  Show this screen
    --version   Show the version of the project

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the detection:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

int main(int argc, const char *argv[]) {
	/// \TODO 
	///  Modify the program syntax and the call to **docopt()** in order to
	///  add options and arguments to the program.
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},	// array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

	std::string input_wav = args["<input-wav>"].asString();
	std::string output_txt = args["<output-txt>"].asString();
  std::string window = args["--window"].asString();
  float val = std::stof(args["--umbral"].asString()); 
  float th_clip = std::stof(args["--clipping"].asString()); 

  //cout << val << '\n'; // Para pruebas

  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;

  // Define analyzer
  PitchAnalyzer analyzer(n_len, rate, PitchAnalyzer::RECT, 50, 550, val, th_clip); //Estos dos últimos los he añadido YO! 

  /// \TODO
  /// Preprocess the input signal in order to ease pitch estimation. For instance,
  /// central-clipping or low pass filtering may be used.
  
  /// \DONE
  /// Hecho en pitch_analyzer.cpp

  // Iterate for each frame and save values in f0 vector
  vector<float>::iterator iX;
  vector<float> f0;
  for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
    float f = analyzer(iX, iX + n_len);
    f0.push_back(f);
  }

  /// \TODO
  /// Postprocess the estimation in order to supress errors. For instance, a median filter
  /// or time-warping may be used.
  vector<float> f0_F = f0; //Creamos un vector copia de f0 para poder extraer los valores ya que los modificaremos directamente de f0

  for(int n = 1; n < f0_F.size() - 1; n++){
    //cout << n << "\n";
    //cout << f0_F[n] << "\n";
    float arr[] = {f0_F[n-1], f0_F[n], f0_F[n+1]};
      for (int j = 0; j<3; j++){
        //cout << arr[j] << " ";
      }
    sort(arr, arr+3);
    f0[n] = arr[1];
    //cout << "\n" << f0[n] << "\n\n";
  }
  /// \DONE
  /// median filter applied

  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; //pitch at t=0
  //os << 0 << '\n'; // OJO!!! HE AÑADIDO ESTA LINEA PORQUE NO CUADRABA LA REFERENCIA CON NUESTRO OUTPUT!!!
  for (iX = f0.begin(); iX != f0.end(); ++iX) 
    os << *iX << '\n';
  os << 0 << '\n';//pitch at t=Dur

  return 0;
}