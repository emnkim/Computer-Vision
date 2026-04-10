# Program 1 - 2D Image Transformation

**Author:** Emily Kim

## Description
This program applies a 2D linear transformation to a GIF image using six parameters: scale, translation, rotation, and shear. It uses inverse mapping with bilinear interpolation to produce a clean output image without holes.

## Files
- `Program1.cpp` — main source file containing all transformation and interpolation logic
- `Image.h` — provided image class (not written by student)
- `test1.gif` — input image (must be in the same directory)
- `output.gif` — output image (written after running the program)

## How to Run
```
Program1.exe sx sy tx ty theta k
```

### Parameters (in order)
| Parameter | Description |
|-----------|-------------|
| `sx` | Scale factor in x |
| `sy` | Scale factor in y |
| `tx` | Translation in x (pixels) |
| `ty` | Translation in y (pixels) |
| `theta` | Rotation angle (degrees) |
| `k` | Shear factor |

### Example Commands
```
# Rotation by 20 degrees
Program1.exe 1.0 1.0 0 0 20 0

# Scale by 1.5 in both x and y
Program1.exe 1.5 1.5 0 0 0 0

# Translation by 20 in x and 40 in y
Program1.exe 1.0 1.0 20 40 0 0

# Shear by 0.5
Program1.exe 1.0 1.0 0 0 0 0.5

# Combination of all of the above
Program1.exe 1.5 1.5 20 40 20 0.5
```

## How It Works
The forward transformation is defined as:
```
q = S * K * R * p + t
```
Where S is scale, K is shear, R is rotation, p is the source pixel, and t is translation.

To avoid holes in the output, the program uses **inverse mapping** — for each output pixel `q`, it computes the corresponding source pixel `p` using:
```
p = R^-1 * K^-1 * S^-1 * (q - t - c) + c
```
Where `c` is the image center (so rotation/scale/shear are applied around the center).

The source pixel color is then computed using **bilinear interpolation** from the four nearest pixels. Output pixels with no corresponding source pixel are left black.

## Assumptions
- Input image is named `test1.gif` and is in the same directory as the executable
- Output is written to `output.gif` in the same directory
- Exactly 6 command line arguments must be provided
