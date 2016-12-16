#define cimg_display 0
#include "CImg.h"
#include "slVector.H"
#include <vector>
#include <string>
#include <iostream>     // std::cout
#include <algorithm>    // std::max
#include <sstream>

using namespace cimg_library;
using namespace std;

// global vars
double** gradience;
double** seamMatrix;
vector<int> seamPath;

// get energy
double getEnergy(SlVector3 l, SlVector3 r, SlVector3 t, SlVector3 d) {
	SlVector2 energyVec2(0, 0);
	SlVector3 horDist = l - r;
	SlVector3 vertDist = t - d;
	energyVec2[0] = mag(horDist);
	energyVec2[1] = mag(vertDist);
	return mag(energyVec2);
}

/* energy is right */
void getEnergyMatrix(SlVector3* image, int width, int height) {

	SlVector3 left, right, top, bottom;
	gradience = new double*[height];
	for (int i = 0; i < height; i++) {
		gradience[i] = new double[width];
	}

	// compute the energy
	for (int col = 0; col < width; col++) {
		for (int row = 0; row < height; row++) {
			int position = col * height + row;
			// find neighbors
			left = image[(col == 0) ? position : position - height];
			right = image[(col == width - 1) ? position : position + height];
			top = image[(row == 0) ? position : position - 1];
			bottom = image[(row == height - 1) ? position : position + 1];
			double value = getEnergy(left, right, top, bottom);
			gradience[row][col] = value;
		}
	}
}

void getVertSeamMatrix(int width, int height) {
	seamMatrix = new double* [height];
	for (int i = 0; i < height; i++) {
		seamMatrix[i] = new double[width];
	}

	// get the min in the first row
	for (int i = 0; i < width; i++) {
		seamMatrix[0][i]= gradience[0][i];
	}

	// find the rest in the matrix
	for (int i = 1; i < height; i++) {
		for (int j = 0; j < width; j++) {
			seamMatrix[i][j] = gradience[i][j];
			if (i == 0) {
				seamMatrix[i][j] += std::min(gradience[i - 1][j + 1], gradience[i - 1][j]);
			} else if (i == width - 1) {
				seamMatrix[i][j] += std::min(gradience[i - 1][j - 1], gradience[i - 1][j]);
			} else {
				double val1 = std::min(gradience[i - 1][j], gradience[i - 1][j + 1]);
				double val2 = std::min(val1, gradience[i - 1][j - 1]);
				seamMatrix[i][j] += val2;
			}
		}
	}

}

SlVector3* transPose(SlVector3* image, int width, int height) {
	SlVector3* newImg = new SlVector3[width * height];

	int newHeight = width;
	int newWidth = height;
	int k = 0;
	for(int i = 0; i < newWidth; i++) {
		for(int j = 0; j < newHeight; j++) {
			int pos = i + j * newWidth;
			newImg[k] = image[pos];
			k++;
		}
	}

	delete[] image;
	image = newImg;
	newImg = NULL;
	return image;
}

// remove vertical seam
SlVector3* removeVerticalSeams(SlVector3* image, int width, int height) {

	int newWidth = width - 1;
	SlVector3* newImg = new SlVector3[newWidth * height];
	// find the min for the last row
	int minCol = 0;
	for (int j = 0; j < width; j++) {
		if (seamMatrix[height - 1][minCol] > seamMatrix[height - 1][j]) {
			minCol = j;
		}
	}

	// dynamic programming, get the min of the previous row
	for (int y = height - 1; y >= 0; y--) {
		bool flag = false;
		for (int x = 0; x < newWidth; x++) {
			if (x == minCol) {
				flag = true;
			}
			// x is col, y is row
			int newCell = x * height + y;
			int oldCell = (flag ? x + 1 : x) * height + y;
			newImg[newCell] = image[oldCell];
		}

		// find the next minCol
		if (y > 0) { // if not in the top of the image
			double midTop = seamMatrix[y - 1][minCol];
			if(minCol > 0 && minCol < width - 1) {
				if(seamMatrix[y - 1][minCol - 1] <= midTop) {
					if(seamMatrix[y - 1][minCol - 1] < seamMatrix[y - 1][minCol + 1]) {
						minCol--;
					} else if(seamMatrix[y - 1][minCol - 1] > seamMatrix[y - 1][minCol + 1]) {
						minCol++;
					}
				}
			} else {
				if(minCol > seamMatrix[y - 1][y + 1]) {
					minCol++;
				}
			}
		}
	}

	delete[] image;
	image = newImg;
	newImg = NULL;
	return image;
}

// delete the allocate energy matrix and seamMatrix
void freeMemory(int height) {
	for(int i = 0; i < height; i++) {
		delete[] seamMatrix[i];
		delete[] gradience[i];
	}
	delete[] seamMatrix;
	delete[] gradience;
}

int main(int argc, char *argv[]) {
	if(argc != 5) {
		cout << "please enter\n./seamcarving <input.jpg> <output.jpg> width height \nto run the program" << endl;
		exit(1);
	}
	// input image here
	CImg<double> input(argv[1]);
	CImg<double> lab = input.RGBtoLab();
	SlVector3 *image = new SlVector3[input.width() * input.height()];

	for (int i = 0; i < input.width(); i++) {
		for (int j = 0; j < input.height(); j++) {
			image[i * input.height() + j][0] = lab(i, j, 0);
			image[i * input.height() + j][1] = lab(i, j, 1);
			image[i * input.height() + j][2] = lab(i, j, 2);
		}
	}

	cout << "image is: " << input.width() << " " << input.height() << endl;

	// drake testing
	// get total gradience
	int width = input.width();
	int height = input.height();

	string outputH = argv[4];
	string outputW = argv[3];

	// convert to string
	istringstream ss(outputH);
	int outHeight = 0;
	ss >> outHeight;
	ss.clear();
	ss.str(outputW);
	int outWidth = 0;
	ss >> outWidth;
	ss.clear();

	cout << outWidth << " " << outHeight << endl;
	int loopWidth = width - outWidth;
	int loopHeight = height - outHeight;
	if(loopWidth < 0 || loopHeight < 0) {
		cout << "Can't enlarge image yet, exit" << endl;
		exit(1);
	}
	cout << "start vertical seam trim, the difference is: " << loopWidth << endl;
	if(loopWidth != 0) {
		for(int i = 0; i < loopWidth; i++) {
			getEnergyMatrix(image, width, height);
			getVertSeamMatrix(width, height);
			image = removeVerticalSeams(image, width, height);
			freeMemory(height);
			width--;
		}
	}

	cout << "finish vertseam" << endl;

	image = transPose(image, width, height);
	cout << "start horizontal seam trim, the difference is: " << loopHeight << endl;
	int width_n = height;
	int height_n = width;
	if(loopHeight != 0) {
		//image = reTransPose(image, height, width);
		for(int i = 0; i < loopHeight; i++) {
			getEnergyMatrix(image, width_n, height_n);
			getVertSeamMatrix(width_n, height_n);
			image = removeVerticalSeams(image, width_n, height_n);
			freeMemory(height_n);
			width_n--;
		}
	}
	cout << "finish the horizontal seam" << endl;
	image = transPose(image, width_n, height_n);

	CImg<double> output(atoi(argv[3]), atoi(argv[4]), input.depth(), input.spectrum(), 0);
	for (int i = 0; i < output.width(); i++) {
		for (int j = 0; j < output.height(); j++) {
			output(i, j, 0) = image[i * output.height() + j][0];
			output(i, j, 1) = image[i * output.height() + j][1];
			output(i, j, 2) = image[i * output.height() + j][2];
		}
	}

	CImg<double> rgb = output.LabtoRGB();
	if (strstr(argv[2], "png"))
		rgb.save_png(argv[2]);
	else if (strstr(argv[2], "jpg"))
		rgb.save_jpeg(argv[2]);

	return 0;
}
