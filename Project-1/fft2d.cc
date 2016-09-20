// Distributed two-dimensional Discrete FFT transform
// VIGNESH SRIDHARAN
// ECE8893 Project 1


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <signal.h>
#include <math.h>
#include <mpi.h>

#include "Complex.h"
#include "stdlib.h"
#include "InputImage.h"

using namespace std;


//void Transform2D(const char* inputFN) 
//{ // Do the 2D transform here.
  // 1) Use the InputImage object to read in the Tower.txt file and
  //    find the width/height of the input image.
  // 2) Use MPI to find how many CPUs in total, and which one
  //    this process is
  // 3) Allocate an array of Complex object of sufficient size to
  //    hold the 2d DFT results (size is width * height)
  // 4) Obtain a pointer to the Complex 1d array of input data
  // 5) Do the individual 1D transforms on the rows assigned to your CPU
  // 6) Send the resultant transformed values to the appropriate
  //    other processors for the next phase.
  // 6a) To send and receive columns, you might need a separate
  //     Complex array of the correct size.
  // 7) Receive messages from other processes to collect your columns
  // 8) When all columns received, do the 1D transforms on the columns
  // 9) Send final answers to CPU 0 (unless you are CPU 0)
  //   9a) If you are CPU 0, collect all values from other processors
  //       and print out with SaveImageData().
//  InputImage image(inputFN);  // Create the helper object for reading the image
  // Step (1) in the comments is the line above.
  // Your code here, steps 2-9
//}

//Implementation of 1D DFT using formula provided in handout
void Transform1D(Complex* h, int w, Complex* H)
{
  for (int n = 0; n < w; n++) {
    H[n] = Complex(0.0); 
    for (int k = 0; k < w; k++) {
	H[n] = H[n] + (h[k] * Complex(+cos((2 * M_PI * k * n)/w), -sin((2 * M_PI * k * n)/w)));
    }
    //if (fabs(H[i].imag) < 1e-10) H[i].imag = 0;
    //if (fabs(H[i].real) < 1e-10) H[i].real = 0;
  }    
}

void Transform2D(Complex *h, int w, Complex* H)
{
  //Implemented simply after performing transpose of the 1D DFT result
  Transform1D(h,w,H);
}

void Transpose2D(Complex *h, int w)
{
  Complex temp;
  for(int i = 0; i < w; i++) {
    for(int k = ((i*w)+i + 1); k < (i+1) *w; k++) {
  temp = h[k];
  h[k] = h[ (k - (i*w)) * w + i];
  h[(k-i*w)*w + i] = temp;
    }
  }
}

void InverseTransform1D(Complex* h, int w, Complex* H)
{

  for (int n = 0; n < w; n++) {
    H[n] = Complex(0.0); 
    for (int k = 0; k < w; k++) {
    	double realcomp = (cos((2*M_PI*k*n)/w))/w;
        double imagcomp = (sin((2*M_PI*k*n)/w))/w;
        H[n] = H[n] + (h[k] * Complex(+realcomp, +imagcomp));
    }
    
  }    
    
}

void InverseTransform2D(Complex *h, int w, Complex* H)
{
  //Implemented simply after performing transpose of the 1D DFT result
  InverseTransform1D(h,w,H);
}

int main(int argc, char** argv)
{
  	string fn("Tower.txt"); // default file name
  	if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line

  	InputImage image(fn.c_str());
  	int h = image.GetHeight();
  	int w = image.GetWidth();
  	//long int imgPixels =(long int)(h * w);

  	
  	int numtasks, rank, rc;// MPI initialization here
  
	rc = MPI_Init(&argc, &argv);
	if(rc != MPI_SUCCESS) {
		printf("Error starting MPI program. Terminating \n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

  	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  	Complex *origimg;
  	Complex *img1D = new Complex[h * w];
  	Complex *img2D = new Complex[h * w];
  	Complex *imginv1D = new Complex[h * w];
  	Complex *imginv2D = new Complex[h * w];

  	origimg = image.GetImageData();
 
// Transform 2D
  int rowsPerCpu = h / numtasks;
  int startRowInd = (rowsPerCpu * rank * w);
  
  Complex *in = &origimg[startRowInd];
  Complex *out = &img1D[startRowInd];
  for(int i = 0; i < rowsPerCpu; i++) {
  	Transform1D(in, w, out);
	in += h;
	out += h;
  }

// MPI_Send
  rc = MPI_Send(&img1D[startRowInd], sizeof(Complex)*w*rowsPerCpu, MPI_CHAR, 0, rank ,MPI_COMM_WORLD);
  if(rc != MPI_SUCCESS)
    {
      cout<< "Send from CPU = "<<rank<<" failed";
      MPI_Finalize();
      exit(1);
    }

  MPI_Status status;

  if(rank == 0) {
    for(int i = 0; i <numtasks; i++) {
	rc = MPI_Recv(&img1D[rowsPerCpu * i * w], sizeof(Complex)*w*rowsPerCpu, MPI_CHAR, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
     }
    image.SaveImageData("MyAfter1D.txt", img1D, h, w);

    Transpose2D(img1D,w);

    for(int i = 1; i < numtasks; i++) {
       rc = MPI_Send(img1D, sizeof(Complex)*w*h, MPI_CHAR, i , i ,MPI_COMM_WORLD);
       if(rc != MPI_SUCCESS)
       {
         cout<< "Send to CPU = "<<i<<" failed";
         MPI_Finalize();
         exit(1);
       }
    }  
  }

  if(rank != 0) { 
  rc = MPI_Recv(img1D, sizeof(Complex)*w*h, MPI_CHAR, 0, rank, MPI_COMM_WORLD, &status);
  if(rc != MPI_SUCCESS)
    {
        cout<< "Recieved to CPU = "<<rank<<" failed";
        MPI_Finalize();
        exit(1);
    }
  }
  
  in = &img1D[startRowInd];
  out = &img2D[startRowInd];
  for(int i = 0; i < rowsPerCpu; i++) {
  	Transform2D(in, w, out);
	in += h;
	out += h;
  }

  rc = MPI_Send(&img2D[startRowInd], sizeof(Complex)*w*rowsPerCpu, MPI_CHAR, 0, rank ,MPI_COMM_WORLD);
  if(rc != MPI_SUCCESS)
    {
      cout<< "Send from CPU = "<<rank<<" failed";
      MPI_Finalize();
      exit(1);
    }

  if(rank == 0) {
    for(int i = 0; i < numtasks; i++) {
	rc = MPI_Recv(&img2D[rowsPerCpu * i * w], sizeof(Complex)*w*rowsPerCpu, MPI_CHAR, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
        if(rc != MPI_SUCCESS)
         {
          cout<< "Transform 1D: Recieve to CPU 0 from CPU = "<<i<<" failed";
          MPI_Finalize();
          exit(1);
         }
    }
    Transpose2D(img2D, w);
    image.SaveImageData("MyAfter2D.txt", img2D, h, w);
  }


// Inverse 2D Transform

//Sending the data from CPU 0 to all others to compute Inverse 1D transform
  
  if(rank == 0) {
    for(int i = 1; i <numtasks; i++) {
      rc = MPI_Send(img2D, sizeof(Complex)*w*h, MPI_CHAR, i , i ,MPI_COMM_WORLD);
      if(rc != MPI_SUCCESS)
       {
        cout<< "Send to CPU = "<<i<<" failed";
        MPI_Finalize();
        exit(1);
       }
    }  
  }

  if(rank != 0) {
   rc = MPI_Recv(img2D, sizeof(Complex)*w*h, MPI_CHAR, 0, rank, MPI_COMM_WORLD, &status);
   if(rc != MPI_SUCCESS)
    {
        cout<< "Inverse 1D: Recieved to CPU = "<<rank<<" failed";
        MPI_Finalize();
        exit(1);
    }
  }
    
  in = &img2D[startRowInd];
  out = &imginv1D[startRowInd];
  for(int i = 0; i < rowsPerCpu; i++) {
  	InverseTransform1D(in, w, out);
	in += h;
	out += h;
  }

// Sending Data from all other CPU's to CPU 0

  rc = MPI_Send(&imginv1D[startRowInd], sizeof(Complex)*w*rowsPerCpu, MPI_CHAR, 0, rank ,MPI_COMM_WORLD);
  if(rc != MPI_SUCCESS)
    {
      cout<< "Send from CPU = "<<rank<<" failed";
      MPI_Finalize();
      exit(1);
    }

// CPU 0 will recieve all the data 
  if(rank == 0) {
    for(int i = 0; i <numtasks; i++) {
	rc = MPI_Recv(&imginv1D[rowsPerCpu * i * w], sizeof(Complex)*w*rowsPerCpu, MPI_CHAR, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
        if(rc != MPI_SUCCESS)
         {
          cout<< "Inverse 1D: Recieve to CPU 0 from CPU = "<<i<<" failed";
          MPI_Finalize();
          exit(1);
        }
    }
    Transpose2D(imginv1D,w);
    for(int i = 1; i <numtasks; i++) {
      rc = MPI_Send(imginv1D, sizeof(Complex)*w*h, MPI_CHAR, i , i ,MPI_COMM_WORLD);
      if(rc != MPI_SUCCESS)
       {
        cout<< "Inverse 1D: Send from CPU 0 to CPU = "<<i<<" failed";
        MPI_Finalize();
        exit(1);
       }
    }  
  }

  if(rank != 0) {
   rc = MPI_Recv(imginv1D, sizeof(Complex)*w*h, MPI_CHAR, 0, rank, MPI_COMM_WORLD, &status);
   if(rc != MPI_SUCCESS)
    {
        cout<< "Inverse 1D: Recieved to CPU = "<<rank<<" failed";
        MPI_Finalize();
        exit(1);
    }
  }

  in = &imginv1D[startRowInd];
  out = &imginv2D[startRowInd];
  for(int i = 0; i < rowsPerCpu; i++) {
  	InverseTransform2D(in, w, out);
	in += h;
	out += h;
  }

//  InverseTransform2D(imginv1D, w, imginv2D, numtasks, rank);

  rc = MPI_Send(&imginv2D[startRowInd], sizeof(Complex)*w*rowsPerCpu, MPI_CHAR, 0, rank ,MPI_COMM_WORLD);
  if(rc != MPI_SUCCESS)
    {
      cout<< "Inverse 2D: Send to CPU 0 from CPU = "<<rank<<" failed";
      MPI_Finalize();
      exit(1);
    }

  if(rank == 0) {
    for(int i = 0; i <numtasks; i++) {
	rc = MPI_Recv(&imginv2D[rowsPerCpu * i * w], sizeof(Complex)*w*rowsPerCpu, MPI_CHAR, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
        if(rc != MPI_SUCCESS)
         {
          cout<< "Inverse 2D: Recieve to CPU 0 from CPU = "<<i<<" failed";
          MPI_Finalize();
          exit(1);
        }
     }
    Transpose2D(imginv2D, w);
    image.SaveImageData("MyAfterInverse.txt", imginv2D, h, w);
  }

  MPI_Finalize();

}  
  

  
