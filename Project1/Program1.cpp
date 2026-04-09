#include "Program1.h"
#include "Image.h"
#include <cmath>
#include <iostream>

using namespace std;

/*
Invert a 2x2 matrix

[x1  y1]^-1 = 1/(x1y2-y1x2) * [x2  -y1]
[x2  y2]                      [-x2   x1]
*/
Matrix invertMatrix(Matrix m) {
    double x1 = m[0][0], y1 = m[0][1];
    double x2 = m[1][0], y2 = m[1][1];

    double inverse = 1/(x1*y2 - y1*x2);
    Matrix res = {{inverse * x2, inverse * -y1}, {inverse * -y2, inverse * x1}};

    return res;
}

/*
Matrix multiplied by a point

[x1  y1] * [px] = [x1px  y1py]
[x2  y2] * [py]   [x2px  y2py]
*/
Point multiplyPoint(Matrix m, Point p) {
    Point res(2);

    res[0] = m[0][0]*p[0] + m[0][1]*p[1];
    res[1] = m[1][0]*p[0] + m[1][1]*p[1];

    return res;
}

/*
Matrix multiplied by a matrix

[m1x1  m1y1] * [m2x1  m2y1] = [m1x1 * m2x1 + m1y1 * m2x2    m1x1 * m2y1 + m1y1 * m2y2]
[m1x2  m1y2] * [m2x2  m2y2]   [m1x2 * m2x1 + m1y2 * m2x2    m1x2 * m2y1 + m1y2 * m2y2]
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
Computes p 

p = R^-1 K^-1 S^-1 (q - t - c) + c

Must multiply backwards: S * K * R * (q - t - c)
*/
Matrix transform(double sx, double sy, double tx, double ty, double theta, double k, Point q, Point center) {
    Point t = {{tx, ty}};
    Matrix R = {{cos(theta), -sin(theta)}, {sin(theta), cos(theta)}};
    Matrix K = {{1, k}, {0, 1}};
    Matrix S = {{sx, 0}, {0, sy}};

    //Inverse of all Matrixes
    Matrix R_inv = invertMatrix(R);
    Matrix K_inv = invertMatrix(K);
    Matrix S_inv = invertMatrix(S);

    //Parenthesis Point Math
    Point qtc(2);
    qtc[0] = q[0] - tx - center[0];
    qtc[1] = q[1] - ty - center[1];

    //Backwards multiplaction of inverse Matrixes
    Matrix SK = multiplyMatrix(K_inv, S_inv);
    Matrix SKR = multiplyMatrix(SK, R_inv);

    Point SKR_qtc = multiplyPoint(SKR, qtc);

    SKR_qtc[0] += center[0];
    SKR_qtc[1] += center[1];

    return SKR_qtc;
}

/*
Computes Bilinear Interpolation using this equation:
    (1−α)(1−β)I(r,c) + α(1−β)I(r+1,c) + (1−α)βI(r,c+1) + αβI(r+1,c+1)
        Top Left        Bottom Left       Top Right      Bottom Right
*/
double bilinear(Image& img, int width, int height, int channels, double x, double y, int c) {
    //Corner of pixel
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;


    double a = x - x0;
    double b = y - y0;

    //Image bounds
    x0 = max(0, min(x0, width - 1));
    x1 = max(0, min(x1, width - 1));
    y0 = max(0, min(y0, height - 1));
    y1 = max(0, min(y1, height - 1));

    //Pixel values for each corner
    double TL = img[(y0 * width + x0) * channels + c];
    double BL = img[(y0 * width + x1) * channels + c];
    double TR = img[(y1 * width + x0) * channels + c];
    double BR = img[(y1 * width + x1) * channels + c];

    return (1-a)*(1-b)*TL + a*(1-b)*BL + (1-a)*b*TR + a*b*BR;
}

int main(int argc, char *argv[]) {
    double sx, sy, tx, ty, theta, k;
    sscanf_s(argv[1], "%lf", &sx);
    sscanf_s(argv[2], "%lf", &sy);
    sscanf_s(argv[3], "%lf", &tx);
    sscanf_s(argv[4], "%lf", &ty);
    sscanf_s(argv[5], "%lf", &theta);
    sscanf_s(argv[6], "%lf", &k);

    theta = theta * M_PI / 180.0;

    //Load
    Image input("test1.gif");
    int width = input.Width();
    int height = input.Height();
    int channels = input.Channels();

    //Allocate output image (black by default)
    Image output(width, height, channels);

    //Center of image
    Point center(2);
    center[0] = width  / 2.0;
    center[1] = height / 2.0;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            Point q(2);
            q[0] = col;
            q[1] = row;

            Point p = applyTransform(sx, sy, tx, ty, theta, k, q, center);

            double px = p[0];
            double py = p[1];

            //If within bounds, bilinear interpolate
            if (px >= 0 && px < width - 1 && py >= 0 && py < height - 1) {
                for (int c = 0; c < channels; c++) {
                    double val = bilinear(input, width, height, channels, px, py, c);
                    output[(row * width + col) * channels + c] = static_cast<unsigned char>(val);
                }
            }
            //Leave black
        }
    }

    output.Save("output.gif");
    cout << "Output written to output.gif" << endl;

    return 0;
}