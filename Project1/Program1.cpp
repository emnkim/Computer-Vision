#define _USE_MATH_DEFINES // for M_PI

/*
File: Program1.cpp
Author: Emily Kim 
Description: This program applies a 2D linear transformation to an image using
             six parameters: scale (sx, sy), translation (tx, ty), rotation (theta),
             and shear (k). The transformation is applied using inverse mapping with
             bilinear interpolation.

Assumptions: Input image is named "test1.gif" and is in the same directory.
             Output image is written to "output.gif".
             Program is run with exactly 6 command line arguments: sx sy tx ty theta k
*/

#include "Image.h"
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm> // for max and min

using namespace std;

typedef vector<double> Point;
typedef vector<vector<double>> Matrix;

/*
    Purpose: Inverts a 2x2 matrix.
    Pre-conditions: Matrix m is a 2x2 matrix with a non-zero determinant.
    Post-conditions: Returns the inverse of m.

    The equation used to invert a 2x2 matrix [[x1, y1], [x2, y2]] is:

    [x1  y1]^-1  =  1/(x1y2-y1x2) * [ y2  -y1]
    [x2  y2]                        [-x2   x1]
*/
Matrix invertMatrix(Matrix m) {
    double x1 = m[0][0], y1 = m[0][1];
    double x2 = m[1][0], y2 = m[1][1];

    double det = x1 * y2 - y1 * x2;
    double inverse = 1.0 / det;

    Matrix res = {{inverse * y2, inverse * -y1}, {inverse * -x2, inverse * x1}};

    return res;
}

/*
    Purpose: Multiplies a 2x2 matrix by a 2D point.
    Pre-conditions: m is a 2x2 matrix, p is a 2D point.
    Post-conditions: Returns the resulting 2D point.

    Visualization of the multiplication:
    [x1  y1] * [px] = [x1*px + y1*py]
    [x2  y2]   [py]   [x2*px + y2*py]
*/
Point multiplyPoint(Matrix m, Point p) {
    Point res(2);

    res[0] = m[0][0] * p[0] + m[0][1] * p[1];
    res[1] = m[1][0] * p[0] + m[1][1] * p[1];

    return res;
}

/*
    Purpose: Multiplies two 2x2 matrices together.
    Pre-conditions: m1 and m2 are both 2x2 matrices.
    Post-conditions: Returns the resulting 2x2 matrix.

    Visualization of the multiplication:
    [m1x1  m1y1] * [m2x1  m2y1] = [m1x1*m2x1 + m1y1*m2x2    m1x1*m2y1 + m1y1*m2y2]
    [m1x2  m1y2]   [m2x2  m2y2]   [m1x2*m2x1 + m1y2*m2x2    m1x2*m2y1 + m1y2*m2y2]
*/
Matrix multiplyMatrix(Matrix m1, Matrix m2) {
    Matrix res(2, vector<double>(2));

    res[0][0] = m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0];
    res[0][1] = m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1];
    res[1][0] = m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0];
    res[1][1] = m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1];

    return res;
}

/*
    Purpose: Computes the inverse transformation to map an output pixel (q) back to its corresponding source pixel (p) in the input image.
    Pre-conditions: sx and sy are non-zero scale factors. theta is in radians. q is a 2D point in the output image.
    Post-conditions: Returns the corresponding source point p in the input image.

    Uses the formula: 
        p = R^-1 K^-1 S^-1 (q - t - c) + c

*/
Point transform(double sx, double sy, double tx, double ty, double theta, double k, Point q, Point center) {
    Matrix R = {{cos(theta), -sin(theta)}, {sin(theta), cos(theta)}};
    Matrix K = {{1, k}, {0, 1}};
    Matrix S = {{sx, 0}, {0, sy}};

    Matrix R_inv = invertMatrix(R);
    Matrix K_inv = invertMatrix(K);
    Matrix S_inv = invertMatrix(S);

    //Combine as R^-1 * K^-1 * S^-1
    Matrix RK_inv  = multiplyMatrix(R_inv, K_inv);
    Matrix RKS_inv = multiplyMatrix(RK_inv, S_inv);

    //(q - t - c)
    Point qtc(2);
    qtc[0] = q[0] - tx - center[0];
    qtc[1] = q[1] - ty - center[1];

    Point result = multiplyPoint(RKS_inv, qtc);
    result[0] += center[0];
    result[1] += center[1];

    return result;
}

/*
    Purpose: Computes the bilinear interpolated pixel value at a fractional (x, y) position in the image for a given channel c.
    Pre-conditions: img is a valid image. x and y are within or near image bounds. c is 0 (red), 1 (green), or 2 (blue).
    Post-conditions: Returns the interpolated pixel value clamped to [0, 255].

    Computes Bilinear Interpolation using this equation:
            (1−α)(1−β)I(r,c) + α(1−β)I(r+1,c) + (1−α)βI(r,c+1) + αβI(r+1,c+1)
                Top Left        Bottom Left       Top Right      Bottom Right
*/
double bilinear(Image& img, int width, int height, double x, double y, int c) {
    //Pixel corners
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    double alpha = x - x0;
    double beta = y - y0;

    x0 = max(0, min(width - 1,  x0));
    x1 = max(0, min(width - 1,  x1));
    y0 = max(0, min(height - 1, y0));
    y1 = max(0, min(height - 1, y1));

    //Get pixel values at each corner
    pixel TL = img.getPixel(y0, x0);
    pixel TR = img.getPixel(y0, x1);
    pixel BL = img.getPixel(y1, x0);
    pixel BR = img.getPixel(y1, x1);

    double tl, tr, bl, br;
    if (c == 0) {
        tl = (double)TL.red;
        tr = (double)TR.red;
        bl = (double)BL.red;
        br = (double)BR.red;
    } else if (c == 1) {
        tl = (double)TL.green;
        tr = (double)TR.green;
        bl = (double)BL.green;
        br = (double)BR.green;
    } else {
        tl = (double)TL.blue;
        tr = (double)TR.blue;
        bl = (double)BL.blue;
        br = (double)BR.blue;
    }

    double result = (1-alpha)*(1-beta)*tl + alpha*(1-beta)*tr + (1-alpha)*beta*bl + alpha*beta*br;

    return max(0.0, min(255.0, result));
}

/*
    Purpose: Computes the pixel and writes the interpolated color to the output image.
    Pre-conditions: Input is a valid loaded image. output is allocated with same dimensions.
    Post-conditions: Output is filled with transformed pixels, black where out of bounds.
*/
void applyTransform(Image& input, Image& output, int width, int height, double sx, double sy, double tx, double ty, double theta, double k) {
    Point center(2);
    center[0] = width  / 2.0;
    center[1] = height / 2.0;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            Point q(2);
            q[0] = col;
            q[1] = row;

            Point p = transform(sx, sy, tx, ty, theta, k, q, center);

            double px = p[0];
            double py = p[1];

            //If within bounds, bilinear interpolate. Else leave black.
            if (px >= 0 && px < width && py >= 0 && py < height) {
                byte r = static_cast<byte>(round(bilinear(input, width, height, px, py, 0)));
                byte g = static_cast<byte>(round(bilinear(input, width, height, px, py, 1)));
                byte b = static_cast<byte>(round(bilinear(input, width, height, px, py, 2)));
                output.setPixel(row, col, r, g, b);
            }
        }
    }
}

/*
    Purpose: Reads transformation parameters, loads input image, applies transformation, and writes output image.
    Pre-conditions: Exactly 6 command line arguments: sx sy tx ty theta k
    Post-conditions: Writes image to "output.gif"
*/
int main(int argc, char *argv[]) {
    //Validate argument count
    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " sx sy tx ty theta k" << endl;
        return 1;
    }

    double sx, sy, tx, ty, theta, k;
    sscanf_s(argv[1], "%lf", &sx);
    sscanf_s(argv[2], "%lf", &sy);
    sscanf_s(argv[3], "%lf", &tx);
    sscanf_s(argv[4], "%lf", &ty);
    sscanf_s(argv[5], "%lf", &theta);
    sscanf_s(argv[6], "%lf", &k);

    //Degrees to radians
    theta = theta * M_PI / 180.0;

    //Load
    Image input("test1.gif");
    int height = input.getRows();
    int width  = input.getCols();

    //Allocate output image (black by default)
    Image output(height, width);

    applyTransform(input, output, width, height, sx, sy, tx, ty, theta, k);

    output.writeImage("output.gif");
    cout << "Output written to output.gif" << endl;

    return 0;
}