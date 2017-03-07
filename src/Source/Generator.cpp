#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>

#include "../Header/Generator.hpp"
#include "../Header/Filesystem.hpp"


using namespace std;
using namespace cv;
/*
The amount of images generated by using all methods in succession is given by:
	#ground_truth * #scales	* #rotations * #perspectives * #backgroundImages * #blures * #noises
	Currently given by: 20 * 3 * 8 * 15 * 10 * 3 * 3 = 648000
*/
Generator::Generator(const string source, const string dest)
	: source(source), dest(dest) {
	cout << "Reading all Images in " << source << " ..." << endl;
	workingFiles = FileSystem::allImagesAtPath(source);

	cout << "Found the following image Files: " << endl;
	for (auto it : workingFiles) {
		cout << it << endl;
	}
	cout << endl;
}

void Generator::border() {
	cout << "Generating ground truth images with border.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "01_border");


	for (auto path : workingFiles) {
		Mat image = fs.loadImage(path);
		cvtColor(image, image, CV_BGR2GRAY);

		int borderSize = image.cols * 0.25;
		Mat borderImage(image.cols + 2 * borderSize, image.rows + 2 * borderSize, image.type());
		borderImage.setTo(Scalar(255, 255, 255));
		image.copyTo(borderImage(Rect(borderSize, borderSize, image.cols, image.rows)));

		string filename = fs.toFileName(path) + "-b" + to_string(borderSize) + fs.toExtension(path, true);
		fs.saveImage(saveFolder, filename, borderImage);

		generated.push_back(fs.toPath(saveFolder, filename));
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " border images." << endl << endl;

	workingFiles = generated;
}

// MAYBE: Generate non-uniform scales.
void Generator::scale() {
	cout << "Generating scaled images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "02_scale");

	for (auto path : workingFiles) {
		Mat image = fs.loadImage(path);
		cvtColor(image, image, CV_BGR2GRAY);
		Mat scaledImage;
		Size scaled = image.size();

		for (int i = 0; i < 3; i++) {
			auto interpolationType = INTER_NEAREST;
			string interpolationID = "-inear";

			if (i == 1) {
				interpolationType = INTER_LINEAR;
				interpolationID = "-ilinear";
			}
			else if (i == 2) {
				interpolationType = INTER_AREA;
				interpolationID = "-iarea";
			}

			// TODO: Experiment with different scale values and step sizes. 
			//For now just scale with 6.0f, to decrease amount of generated images
			for (float scale = 6.0f; scale < 6.1f; scale += (2.0f / 3.0f)) {

				resize(image, scaledImage, Size(), scale, scale, interpolationType);

				string s = to_string(scale).substr(0, 4) + interpolationID;
				string filename = fs.toFileName(path) + "-s" + s + fs.toExtension(path, true);
				fs.saveImage(saveFolder, filename, scaledImage);

				generated.push_back(fs.toPath(saveFolder, filename));
			}
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " scaled images." << endl << endl;

	workingFiles = generated;
}

void Generator::rotate() {
	cout << "Generating rotated images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "03_rotate");

	float step_size = 45.0f;
	float max_rotation = 360.0f;

	int desiredFiles = 200;	//set 0 for to get all possible files
	int estimatedFiles = workingFiles.size() * (max_rotation / step_size);
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}

	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	for (float degree = step_size; degree < max_rotation; degree += step_size) {
		shuffle();
		for (auto path : workingFiles) {

			if (generated.size() >= desiredFiles)
				break;

			if (degree > 360)
				break;

			count++;
			if (count % skip)
				continue;


			Mat image = fs.loadImage(path);
			cvtColor(image, image, CV_BGR2GRAY);
			Mat rotatedImage;
			Point2f image_center(image.cols / 2.0F, image.rows / 2.0F);

			Mat rot_mat = getRotationMatrix2D(image_center, degree, 1.0);
			warpAffine(image, rotatedImage, rot_mat, image.size(), 1, 0, cv::Scalar(255));

			string s = to_string(degree).substr(0, to_string(degree).find_last_of("."));
			string filename = fs.toFileName(path) + "-rot" + s + fs.toExtension(path, true);
			fs.saveImage(saveFolder, filename, rotatedImage);

			generated.push_back(fs.toPath(saveFolder, filename));
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " rotated images." << endl << endl;

	workingFiles = generated;
}

//Apply warpPerspective by moving the topLeftCorner of the Input Image by the given step_size in X and Y Direction, and set topRightCorner and bottomLeft correspondingly. BottomRightCorner is fixed.
void Generator::perspective() {
	cout << "Generating warped images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "04_perspective");

	const float step_size = 0.1f;
	const float maxStep = 0.3f;

	int desiredFiles = 500;	//set 0 for all possible files
	int estimatedFiles = workingFiles.size() * pow(((maxStep / step_size) + 1), 2) - 1;
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}
	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	vector<Point2f> vecsrc;
	vector<Point2f> vecdst;

	//Iterative topLeftQuadrant
	for (float stepY = 0; stepY <= maxStep; stepY += step_size) {
		for (float stepX = 0; stepX <= maxStep; stepX += step_size) {

			shuffle();
			for (auto path : workingFiles) {

				//If size of generated Images exceeds desiredFiles -> stop
				if (generated.size() >= desiredFiles)
					break;

				//If both Steps equal 0, the Transformation will be the identity -> Ignore this case
				if (stepX == 0 && stepY == 0) {
					continue;
				}

				//If StepX or StepY larger than maxStep -> Ignore this case
				if (stepX > maxStep || stepY > maxStep) {
					continue;
				}

				count++;
				if (count % skip) {
					continue;
				}

				Mat image = fs.loadImage(path);
				cvtColor(image, image, CV_BGR2GRAY);
				Point2f topLeft(0, 0);
				Point2f topRight(image.cols - 1, 0);
				Point2f bottomLeft(0, image.rows - 1);
				Point2f bottomRight(image.cols - 1, image.rows - 1);

				vecsrc.clear();
				vecsrc.push_back(topLeft);
				vecsrc.push_back(topRight);
				vecsrc.push_back(bottomLeft);
				vecsrc.push_back(bottomRight);


				Point2f stepPointY(0, stepY *image.rows - 1);
				Point2f vectorFromTopLeftToTopRight(topRight - stepPointY);
				Point2f stepPointX = stepX*vectorFromTopLeftToTopRight;

				Point2f warpedTopLeft = stepPointY + stepX*vectorFromTopLeftToTopRight;
				Point2f warpedTopRight = topRight - stepX*vectorFromTopLeftToTopRight;
				Point2f warpedBottomLeft = bottomLeft - stepPointY;

				vecdst.clear();
				vecdst.push_back(warpedTopLeft);
				vecdst.push_back(warpedTopRight);
				vecdst.push_back(warpedBottomLeft);
				vecdst.push_back(bottomRight);

				Mat homographyMatrix = findHomography(vecsrc, vecdst);
				Mat warpedImage = Mat(image.size(), image.type(), Scalar(255));
				warpPerspective(image, warpedImage, homographyMatrix, image.size(), 1, 0, Scalar(255));
				string s = "0." + to_string(cvRound(stepX * 10)) + "X" + "0." + to_string(cvRound(stepY * 10)) + "Y";
				string filename = fs.toFileName(path) + "-p" + s + fs.toExtension(path, true);
				fs.saveImage(saveFolder, filename, warpedImage);

				generated.push_back(fs.toPath(saveFolder, filename));
			}
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " warped images." << endl << endl;

	workingFiles = generated;

}

void Generator::synthetic() {
	string bgDir = source + separator + ".." + separator + "99_bg";
	cout << "Reading all BGImages in " << bgDir << " ..." << endl;
	bgFiles = FileSystem::allImagesAtPath(bgDir);

	cout << "Found the following image Files: " << endl;
	for (auto it : bgFiles) {
		cout << it << endl;
	}
	cout << endl;

	cout << "Generating synthetic images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "05_synthetic");

	int desiredFiles = 100;	//set 0 to get all possible files
	int estimatedFiles = workingFiles.size() * bgFiles.size();
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}

	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	for (auto bgPath : bgFiles) {
		shuffle();
		for (auto path : workingFiles) {

			//If size of generated Images exceeds desiredFiles -> stop
			if (generated.size() >= desiredFiles)
				break;

			count++;
			if (count % skip)
				continue;

			Mat qrImage = fs.loadImage(path);
			Mat bgImage = fs.loadImage(bgPath);
			Mat syntheticImage;

			float maxScale = 700;
			if (bgImage.cols > maxScale || bgImage.rows > maxScale) {
				float scale;
				if (bgImage.cols > bgImage.rows) {
					scale = maxScale / bgImage.cols;
				}
				else {
					scale = maxScale / bgImage.rows;
				}
				resize(bgImage, syntheticImage, Size(), scale, scale, INTER_AREA);
			}
			else {
				syntheticImage = bgImage;
			}

			int size = cvRound(0.6*min(syntheticImage.rows, syntheticImage.cols)); //Size of QRCode in syntheticImage should be approx 60%
			Mat qrResizedImage = Mat(size, size, qrImage.type());
			if (qrImage.rows > size) {
				//Shrink
				resize(qrImage, qrResizedImage, qrResizedImage.size(), 0, 0, INTER_AREA);
			}
			else {
				//Enlarge
				resize(qrImage, qrResizedImage, qrResizedImage.size(), 0, 0, INTER_LINEAR);
			}

			//Randomize the position of the QRImage inside syntheticImage
			int x = rand() % (syntheticImage.cols - size);
			int y = rand() % (syntheticImage.rows - size);
			qrResizedImage.copyTo(syntheticImage(cv::Rect(x, y, size, size)));

			string filename = fs.toFileName(path) + "-syn" + fs.toFileName(bgPath) + fs.toExtension(path, true);
			fs.saveImage(saveFolder, filename, syntheticImage);

			generated.push_back(fs.toPath(saveFolder, filename));
		}
	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " synthetic images." << endl << endl;

	workingFiles = generated;

}

void Generator::blur() {
	cout << "Generating blured images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "06_blured");

	const int step_size = 6;	//Has to be even! StartKernelSize will be step_size + 1
	const int maxStep = 3 * 6 + 1;

	int desiredFiles = 100;		//Set 0 to get all possible files
	int estimatedFiles = workingFiles.size() * (maxStep - 1) / step_size;
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}
	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	//Iterative through kernelSizes
	for (int size = step_size + 1; size <= maxStep; size += step_size) {
		shuffle();
		for (auto path : workingFiles) {
			Mat image = fs.loadImage(path);
			Mat bluredImage;

			//If size of generated Images exceeds desiredFiles -> stop
			if (generated.size() >= desiredFiles)
				break;

			count++;
			if (count % skip)
				continue;

			GaussianBlur(image, bluredImage, Size(size, size), 0, 0);
			string filename = fs.toFileName(path) + "-blur" + to_string(size) + fs.toExtension(path, true);
			fs.saveImage(saveFolder, filename, bluredImage);

			generated.push_back(fs.toPath(saveFolder, filename));
		}

	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " blured images." << endl << endl;
	workingFiles = generated;
}

void Generator::noise() {
	cout << "Generating noisy images.." << endl;
	vector<string> generated;
	FileSystem fs;

	string saveFolder = fs.makeDir(dest, "07_noise");

	const int step_size = 15;
	const int maxStep = 3 * 15;

	int desiredFiles = 100;		//set 0 to get all possible files
	int estimatedFiles = workingFiles.size() * maxStep / step_size;
	cout << "Desired Files: " << desiredFiles << ", Estimated Files: " << estimatedFiles << endl;
	if (desiredFiles == 0 || estimatedFiles < desiredFiles) {
		desiredFiles = estimatedFiles;
	}

	int skip = estimatedFiles / desiredFiles;
	int count = 0;

	//Iterative through standardDeviations
	for (int stddev = step_size; stddev <= maxStep; stddev += step_size) {
		shuffle();
		for (auto path : workingFiles) {
			Mat image = fs.loadImage(path);

			Mat noise(image.size(), image.type());
			Mat noiseImage;

			//If size of generated Images exceeds desiredFiles -> stop
			if (generated.size() >= desiredFiles)
				break;

			count++;
			if (count % skip)
				continue;

			randn(noise, Scalar::all(0), Scalar::all(stddev));
			addWeighted(image, 1.0, noise, 1.0, 0.0, noiseImage);


			string filename = fs.toFileName(path) + "-noise" + to_string(stddev) + fs.toExtension(path, true);
			fs.saveImage(saveFolder, filename, noiseImage);

			generated.push_back(fs.toPath(saveFolder, filename));
		}

	}

	for (auto path : generated) {
		cout << path << endl;
	}
	cout << "Generated " << to_string(generated.size()) << " noise images." << endl << endl;
	workingFiles = generated;
}

void Generator::shuffle() {
	random_shuffle(workingFiles.begin(), workingFiles.end());
}