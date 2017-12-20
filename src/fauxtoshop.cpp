// This is the CPP file you will edit and turn in.
// Also remove these comments here and add your own.
// TODO: rewrite this comment

#include <iostream>
#include <cstdlib>
#include "console.h"
#include "gwindow.h"
#include "grid.h"
#include "simpio.h"
#include "strlib.h"
#include "gbufferedimage.h"
#include "gevents.h"
#include "math.h" //for sqrt and exp in the optional Gaussian kernel
using namespace std;

static const int    WHITE = 0xFFFFFF;
static const int    BLACK = 0x000000;
static const int    GREEN = 0x00FF00;
static const double PI    = 3.14159265;

static void     doFauxtoshop(GWindow &gw, GBufferedImage &img);
static void     edgeDetection(GBufferedImage &img);
static void     scatter(GBufferedImage &img);
static void     flipVertical(GBufferedImage &img);
static void     checkNeighbors(int &t, Grid<int> &o, Grid<int> &e, int &y, int &x, int &r, int &g, int &b);
static int      maxValue(int &r, int &g, int &b, int &zr, int &zg, int &zb);
static void     greenScreen(GBufferedImage &baseImg, GBufferedImage &sticker);

static bool     openImageFromFilename(GBufferedImage &img, string filename);
static bool 	saveImageToFilename(const GBufferedImage &img, string filename);
static void     getMouseClickLocation(int &row, int &col);
static void     obtainLocation(int &row, int &col);
static Grid<int> detGreen(Grid<int> &img, int h, int w);
static void     sToImg(GBufferedImage &bImg, Grid<int> &baseImg, GBufferedImage &sticker, Grid<int> &sGrid, int row, int col);

/* STARTER CODE FUNCTION - DO NOT EDIT
 *
 * This main simply declares a GWindow and a GBufferedImage for use
 * throughout the program. By asking you not to edit this function,
 * we are enforcing that the GWindow have a lifespan that spans the
 * entire duration of execution (trying to have more than one GWindow,
 * and/or GWindow(s) that go in and out of scope, can cause program
 * crashes).
 */


int main() {
    GWindow gw;
    gw.setTitle("Fauxtoshop");
    gw.setVisible(true);
    GBufferedImage img;
    doFauxtoshop(gw, img);
    return 0;
}

/* This is yours to edit. Depending on how you approach your problem
 * decomposition, you will want to rewrite some of these lines, move
 * them inside loops, or move them inside helper functions, etc.
 *
 * TODO: rewrite this comment.
 */
static void doFauxtoshop(GWindow &gw, GBufferedImage &img) {
    cout << "Welcome to Fauxtoshop!" << endl;
    string fileName = "stanford-oval.jpg";

    while(true){
        openImageFromFilename(img, fileName);
        gw.setCanvasSize(img.getWidth(), img.getHeight());
        gw.add(&img,0,0);

        //Prompt user to select image manipulator
        int choice = 0;
        cout << "Which image filter would you like to apply?" << endl;
        cout << "       1 - Scatter" << endl;
        cout << "       2 - Edge Detection" << endl;
        cout << "       3 - Green Screen with another image" << endl;
        cout << "       4 - Compare image with another image" << endl;
        cout << "       5 - Flip vertically" << endl;
        cout << "Your choice: "; cin >> choice;

        while(choice != 1 || choice != 2 || choice != 3 || choice != 4){
            if(choice == 1){
                cout << "Scatter!" << endl;
                scatter(img);
                break;
            } else if(choice == 2){
                cout << "Edge Detection!" << endl;
                edgeDetection(img);
                break;
            } else if(choice == 3){
                cout << "Green Screen!" << endl;
                string auxName;
                GBufferedImage sticker;

                while(true){
                    cout << "Enter sticker JPEG file name (do not include .filetype [.jpg]): "; cin >> auxName;
                    auxName = auxName + ".jpg";
                    if(openImageFromFilename(sticker, auxName)){
                        cout << "File " + auxName + " opened successfully!" << endl;
                        break;
                    } else {
                        cout << "Invalid filename" << endl;
                    }
                }

                greenScreen(img, sticker);
                break;
            }else if(choice == 4){
                cout << "Compare Images" << endl;
                //TODO: compareImage(baseImg, sticker);  getMouseClickLocation(row, col);
                break;
            } else if(choice == 5){
                cout << "Flip Vertically" << endl;
                flipVertical(img);
                break;
            } else {
                cout << "Invalid choice" << endl;
                break;
                //cout << "Your choice: "; cin >> choice;
            }
        }

        char again = ' ';
        while(true){
            cout << "Modify image again? (Y/N): "; cin >> again;
            if(again == 'y' || again == 'Y' || again == 'n' || again == 'N'){
                break;
            } else {
                cout << "Invalid choice" << endl;
                break;
            }
        }
        if(again == 'n' || again == 'N'){
            break;
        }
    }
    cout << "Thanks for using Fauxtoshop" << endl;
    gw.clear();
}


/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Attempts to open the image file 'filename'.
 *
 * This function returns true when the image file was successfully
 * opened and the 'img' object now contains that image, otherwise it
 * returns false.
 */

static bool openImageFromFilename(GBufferedImage& img, string filename) {
    try { img.load(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Attempts to save the image file to 'filename'.
 *
 * This function returns true when the image was successfully saved
 * to the file specified, otherwise it returns false.
 */
static bool saveImageToFilename(const GBufferedImage &img, string filename) {
    try { img.save(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Waits for a mouse click in the GWindow and reports click location.
 *
 * When this function returns, row and col are set to the row and
 * column where a mouse click was detected.
 */
static void getMouseClickLocation(int &row, int &col) {
    GMouseEvent me;
    do {
        me = getNextEvent(MOUSE_EVENT);
    } while (me.getEventType() != MOUSE_CLICKED);
    row = me.getY();
    col = me.getX();
}

/* OPTIONAL HELPER FUNCTION
 *
 * This is only here in in case you decide to impelment a Gaussian
 * blur as an OPTIONAL extension (see the suggested extensions part
 * of the spec handout).
 *
 * Takes a radius and computes a 1-dimensional Gaussian blur kernel
 * with that radius. The 1-dimensional kernel can be applied to a
 * 2-dimensional image in two separate passes: first pass goes over
 * each row and does the horizontal convolutions, second pass goes
 * over each column and does the vertical convolutions. This is more
 * efficient than creating a 2-dimensional kernel and applying it in
 * one convolution pass.
 *
 * This code is based on the C# code posted by Stack Overflow user
 * "Cecil has a name" at this link:
 * http://stackoverflow.com/questions/1696113/how-do-i-gaussian-blur-an-image-without-using-any-in-built-gaussian-functions
 *
 */
static Vector<double> gaussKernelForRadius(int radius) {
    if (radius < 1) {
        Vector<double> empty;
        return empty;
    }
    Vector<double> kernel(radius * 2 + 1);
    double magic1 = 1.0 / (2.0 * radius * radius);
    double magic2 = 1.0 / (sqrt(2.0 * PI) * radius);
    int r = -radius;
    double div = 0.0;
    for (int i = 0; i < kernel.size(); i++) {
        double x = r * r;
        kernel[i] = magic2 * exp(-x * magic1);
        r++;
        div += kernel[i];
    }
    for (int i = 0; i < kernel.size(); i++) {
        kernel[i] /= div;
    }
    return kernel;
}

static void scatter(GBufferedImage &img){

//Ask user for radius until they enter an int between 1 and 40, inclusive
    int radius = 0;
    while(true){
        cout << "Set threshold (integer between 1 and 40): "; cin >> radius;
        if(radius >= 1 && radius <= 40){
           break;
        } else {
            cout << "Invalid threshold" << endl;
        }
    }

//Create new grid the size of the original image
    Grid<int> original = img.toGrid();
    Grid<int> scatteredImage(original.numRows(), original.numCols());

//Loop over empty grid
    for(int row = 0; row < original.numRows(); row++){
        for(int col = 0; col < original.numCols(); col++){

            int randRow, randCol, rowCoef, colCoef;

            while(true){
            //Randomly determines a positive or negative value coefficient
                if((rand() % 10) % 2 == 0){
                    rowCoef = -1;
                } else {
                    rowCoef = 1;
                }

                if((rand() % 10) % 2 == 0){
                    colCoef = -1;
                } else {
                    colCoef = 1;
                }

            //Randomly selects an adjacent magitude between 1 and "radius" away
                randRow = rowCoef*(rand() % radius + 1);
                randCol = colCoef*(rand() % radius + 1);

                if(original.inBounds(row + randRow, col + randCol)){
                    break;
                }
            }
            scatteredImage[row][col] = original[row + randRow][col + randCol];
        }
    }
    img.fromGrid(scatteredImage);
    cout << "Scattering complete" << endl;
}

static void edgeDetection(GBufferedImage &img){

    int threshold, c, red, green, blue;

    while(true){

        cout << "Set threshold (integer between 1 and 20): "; cin >> threshold;

        if(threshold >= 1 && threshold <= 20){
            break;
        } else {
            cout << "Invalid threshold" << endl;
        }
    }

    //Convert from GBufferedImage to Grid
    Grid<int> original = img.toGrid();
    Grid<int> edge(original.numRows(), original.numCols());

    //Cycle through each pixel in the image
    for(int row = 0; row < original.numRows(); row++){
        for(int col = 0; col < original.numCols(); col++){

            //Obtain pixel RGB values from original image
            c = original[row][col];
            GBufferedImage::getRedGreenBlue(c, red, green, blue);

            //Set the current pixel to white to start (assumes not an edge)
            edge[row][col] = WHITE;

            //Loop over all of the adjacent pixels to the center pixel
            checkNeighbors(threshold, original, edge, row, col, red, green, blue);
        }
    }

    //Update GBufferedImage from Grid
    img.fromGrid(edge);
    cout << "Edge detection complete" << endl;
}

static void checkNeighbors(int &t, Grid<int> &o, Grid<int> &e, int &y, int &x, int &r, int &g, int &b){

    //Assumes all adjacent pixels are below threshold to start. Creates RGB trackers.
    bool subT = true;
    int red, green, blue;

    //Puts the current pixel from original in the center of a 3x3 grid, checks neighbors.
    for(int i = -1; i < 1; i++){
        for(int j = -1; j < 1; j++){

            //Prevents attempt to access RGB values of pixels outside of boundary conditions.
            if(o.inBounds(y+i,x+j) && subT && (i != 0 && j != 0)){

                //Get the RGB values from the current adjacent pixel (e.g. top left of center, top of center, etc.)
                int adjPix = o[y+i][x+j];
                GBufferedImage::getRedGreenBlue(adjPix, red, green, blue);

                //Determine the maximum difference in RGB values
                int max = maxValue(r, g, b, red, green, blue);

                //Set the current pixel to black if it has a greater RGB difference from the current neighbor than the threshold.
                if(max > t){
                    e[y][x] = BLACK;

                    //Breaks out of the neighbor-analysis loop once pixel is determined to be an edge.
                    subT = false;
                }
            }
        }
    }
}

static int maxValue(int &r, int &g, int &b, int &zr, int &zg, int &zb){

    vector<int> dist {abs(r-zr), abs(g-zg), abs(b-zb)};

    int max = 0;

    for(int i = 0; i < dist.size(); i++){
        if(max < dist[i]){
            max = dist[i];
        }
    }
    return max;
}

static void flipVertical(GBufferedImage &img){
    //Convert GBufferedImage ro Grid<int>
    Grid<int> original = img.toGrid();
    Grid<int> flipped(original.numRows(), original.numCols());

    //Do Flip
    for(int row = 0; row < original.numRows(); row++){
        for(int col = 0; col < original.numCols(); col++){
            flipped[original.numRows()-1-row][col] = original[row][col];
        }
    }

    //Update GBufferedImage from Grid
    img.fromGrid(flipped);
    cout << "Vertical flip complete" << endl;
}

static void greenScreen(GBufferedImage &baseImg, GBufferedImage &sticker){

    int row, col;
    obtainLocation(row, col);

    //Make an RGB grid out of the sticker image, nullify pixels that were GREEN.
    Grid<int> sGrid = sticker.toGrid();
    Grid<int> baseGrid = baseImg.toGrid();

    sToImg(baseImg, baseGrid, sticker, sGrid, row, col);
    cout << "Overlay complete" << endl;
}

static void obtainLocation(int &row, int &col){
//Determines the centerpoint around which the sticker will be overlayed.

    cout << "Enter the coordinate at which you'd like to center the sticker (-1 for click): " << endl;
    cout << "Row: "; cin >> row;

    if (row == -1){
        cout << "No row entered. Awaiting mouse click..." << endl;
        getMouseClickLocation(row, col);
    } else {
       cout << "Col: "; cin >> col;
    }
    cout << "Coordinate: Row: " << row << ", Col: " << col << endl;
}

static Grid<int> detGreen(Grid<int> &img, int h, int w){

    int c, red, green, blue;
    c = 0; red = 0; green = 0; blue = 0;

    for(int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){
            c = img[i][j];
            GBufferedImage::getRedGreenBlue(c, red, green, blue);
            cout << "Red: " << red << ", Green: " << green << ", Blue: " << blue << endl;
            if(img[i][j] == GREEN){
                img[i][j] = NULL;
                cout << img[i][j] << endl;
            }
        }
    }
    return img;
}

static void sToImg(GBufferedImage &bImg, Grid<int> &baseGrid, GBufferedImage &sticker, Grid<int> &sGrid, int row, int col){
    for(int i = 0; i < sGrid.height(); i++){
        for(int j = 0; j < sGrid.width(); j++){
            if((row+i) < baseGrid.height() && (col+j) < baseGrid.width()){
                if(i == 0 && j == 0){
                    cout << "Color: " << sGrid[i][j] << endl;
                }
                if(/*sticker.getRGB(i,j) == GREEN*/ /*sGrid[i][j] >= 63750 || sGrid[i][j] <= 63740*/ sGrid[i][j] != sGrid[0][0]){ //63744 = "Green"
                    baseGrid[row+i][col+j] = sGrid[i][j]; //sticker[i][j];
                }
           }
        }
    }
    bImg.fromGrid(baseGrid);
}
