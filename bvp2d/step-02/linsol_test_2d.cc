/*
 * Solve 
 *      -Laplace(u) = f (0,1) x (0,1)
 *      u = g on boundary
 * Exact solution
 *       u(x) = sin(2*pi*x) * sin(2*pi*y)
 */
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>

#include "Vector.h"
#include "sparse_matrix.h"
#include "cg_solver.h"
#include "jacobi_solver.h"
#include "sor_solver.h"
#include "ssor_solver.h"

using namespace std;

//------------------------------------------------------------------------------
// Problem definition and exact solution
//------------------------------------------------------------------------------
double xmin = 0, xmax = 1;
double ymin = 0, ymax = 1;

double boundary_value(const double x, const double y)
{
   return 1.0 + sin(2*M_PI*x)*sin(2*M_PI*y);
}

double rhs_value(const double x, const double y)
{
   return pow(2*M_PI,2) * sin(2*M_PI*x) * sin(2*M_PI*y);
}

//------------------------------------------------------------------------------
// Main program
//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
   if (argc < 4)
   {
      cout << "Specify: nx, ny, solver (jacobi, sor, ssor, cg), max_iter\n";
      cout << "Example: " << argv[0] << " 50 50 jacobi 5000\n";
      exit(1);
   }

   const unsigned int nx = atoi(argv[1]), ny = atoi(argv[2]);
   const string method = string(argv[3]);
   unsigned int max_iter = 1000;
   if(argc == 5) max_iter = atoi(argv[4]);

   const double dx = (xmax - xmin) / (nx - 1);
   const double dy = (ymax - ymin) / (ny - 1);
   const unsigned int n = nx*ny;

   const double a0 =  2.0/(dx*dx) + 2.0/(dy*dy);
   const double a1 = -1.0/(dx*dx);
   const double a2 = -1.0/(dy*dy);

   // Construct matrix
   SparseMatrix<double> A(n);
   for(unsigned int i=0; i<nx; ++i)
      for(unsigned int j=0; j<ny; ++j)
      {
         unsigned int c = i + j*nx;
         unsigned int cl = (i-1) + j*nx;
         unsigned int cr = (i+1) + j*nx;
         unsigned int cb = i + (j-1)*nx;
         unsigned int ct = i + (j+1)*nx;

         A.set(c, c , a0); // diagonal element should be first
         if(i > 0)    A.set(c, cl, a1);
         if(i < nx-1) A.set(c, cr, a1);
         if(j > 0)    A.set(c, cb, a2);
         if(j < ny-1) A.set(c, ct, a2);
      }
   A.close();

   // Solution vector
   Vector<double> u(n);
   u      = 0;

   // Fill bc into u
   {
      double x, y;
      unsigned int c;
      for(unsigned int i=0; i<nx; ++i)
      {
         x = xmin + i*dx;

         // j = 0: bottom
         y = ymin;
         c = i + 0*nx;
         u(c) = boundary_value(x,y);

         // j = ny-1: top
         y = ymax;
         c = i + (ny-1)*nx;
         u(c) = boundary_value(x,y);
      }

      for(unsigned int j=0; j<ny; ++j)
      {
         y = ymin + j*dy;

         // i = 0: left
         x = xmin;
         c = 0 + j*nx;
         u(c) = boundary_value(x,y);

         // i = nx-1: right
         x = xmax;
         c = (nx-1) + j*nx;
         u(c) = boundary_value(x,y);
      }
   }

   // Construct right hand side vector
   Vector<double> f(n);
   A.multiply(u,f); // f = A*u

   // f = rhs - f = rhs - A*u
   for(unsigned int j=0; j<ny; ++j)
   {
      const double y = ymin + j*dy;
      for(unsigned int i=0; i<nx; ++i)
      {
         const double x = xmin + i*dx;
         unsigned int c = i + j*nx;
         f(c) = rhs_value(x,y) - f(c);
      }
   }

   // Modify boundary values of f
   {
      unsigned int c;
      for(unsigned int i=0; i<nx; ++i)
      {
         // j = 0: bottom
         c = i + 0*nx;
         f(c) = A(c,c) * u(c);
         A.zero_off_diag(c);

         // j = ny-1: top
         c = i + (ny-1)*nx;
         f(c) = A(c,c) * u(c);
         A.zero_off_diag(c);
      }

      for(unsigned int j=0; j<ny; ++j)
      {
         // i = 0: left
         c = 0 + j*nx;
         f(c) = A(c,c) * u(c);
         A.zero_off_diag(c);

         // i = nx-1: right
         c = (nx-1) + j*nx;
         f(c) = A(c,c) * u(c);
         A.zero_off_diag(c);
      }
   }

   //cout << "A = \n" << A << endl;
   //cout << "f = \n" << f << endl;

   // Create solver object
   const double tol = 1.0e-6;
   const double h = fmin(dx, dy);
   const double omega = 2.0/(1.0 + sin(M_PI*h)); // relaxation factor in SOR
   unsigned int iter = 0;
   if (method == "jacobi")
   {
      JacobiSolver<double> solver(max_iter, tol);
      iter = solver.solve(A, u, f);
   }
   else if (method == "sor")
   {
      SORSolver<double> solver(max_iter, tol, omega);
      iter = solver.solve(A, u, f);
   }
   else if (method == "ssor")
   {
      SSORSolver<double> solver(max_iter, tol, omega);
      iter = solver.solve(A, u, f);
   }
   else if (method == "cg")
   {
      CGSolver<double> solver(max_iter, tol);
      iter = solver.solve(A, u, f);
   }
   else
   {
      cout << "Unknown solver: " << method << "\n";
      exit(1);
   }

   cout << "Convergence tolerance = " << tol << endl;
   cout << "Number of iterations = " << iter << endl;

   // Save solution to file
   string fname = "u.dat";
   ofstream fsol(fname);
   for(unsigned int j=0; j<ny; ++j)
   {
      double y = ymin + j*dy;
      for(unsigned int i=0; i<nx; ++i)
      {
         double x = xmin + i*dx;
         unsigned int c = i + j*nx;
         fsol << x << "  " << y << "  " << u(c) << endl;
      }
      fsol << endl;
   }
   fsol.close ();
   cout << "Saved solution into file " << fname << endl;
}
