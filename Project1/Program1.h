#pragma once

#include <vector> 

using namespace std;

typedef vector<double> Point;
typedef vector<vector<double>> Matrix;

Matrix invertMatrix(Matrix m);
Point multiplyPoint(Matrix m, Point p);
Matrix multiplyMatrix(Matrix m1, Matrix m2);
Matrix transform(double sx, double sy, double tx, double ty, double theta, double k, Point q, Point center);
double bilinear(Image& img, int width, int height, int channels, double x, double y, int c);