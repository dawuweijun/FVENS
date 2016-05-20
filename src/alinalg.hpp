/** @file alinalg.hpp
 * @brief Linear algebra subroutines
 * @author Aditya Kashi
 */

#ifndef __ALINALG_H

#ifndef __AMESH2DH_H
#include <amesh2dh.hpp>
#endif

#ifndef __AEULERFLUXNORMAL_H
#include <aeulerfluxnormal.hpp>
#endif

#define __ALINALG_H

namespace acfd {

/// Solves Ax=b for dense A by Gaussian elimination
void gausselim(amat::Matrix<acfd_real>& A, amat::Matrix<acfd_real>& b, amat::Matrix<acfd_real>& x);

/// Factors A into unit lower triangular matrix and upper triangular matrix in place, with partial pivoting
void LUfactor(amat::Matrix<acfd_real>& A, amat::Matrix<int>& p);

/// Solve LUx = b
/** \param A contains L and U
 * \param p is the permutation array
 * \param b is the RHS
 * \param x will contain the solution
 */
void LUsolve(const amat::Matrix<acfd_real>& A, const amat::Matrix<int>& p, const amat::Matrix<acfd_real>& b, amat::Matrix<acfd_real>& x);

/// Base class for AF (approximate factorization)-type iterative linear solver
class IterativeSolver
{
protected:
	const int nvars;								///< Number of conserved variables
	const UMesh2dh* const m;						///< mesh
	const amat::Matrix<acfd_real>* const res;		///< Vector of `residuals' at previous iteration
public:
	IterativeSolver(const int num_vars, const UMesh2dh* const mesh, const amat::Matrix<acfd_real>* const residual) 
		: nvars(num_vars), m(mesh), res(residual)
	{ }

	/// Supposed to carry a single step of the corresponding AF solver and store the correction in the argument
	virtual void compute_update(amat::Matrix<acfd_real>* const deltau) = 0;

	virtual ~IterativeSolver()
	{ }
};

/// Base class for matrix-free iterative solvers of AF-type
/** A FluxFunction object allows us to compute inviscid flux corresponding to a given state vector.
 * This is used to compute the action of the lower/upper parts of the (approximate) Jacobian on a given vector.
 * Adapted from reference: 
 * H. Luo, D. Sharov, J.D. Baum, R. Loehner. "On the Computation of Compressible Turbulent Flows on Unstructured Grids". Intl. Journal of Computational Fluid Dynamics Vol 14, No 4, pp 253-270. 2001.
 */
class MatrixFreeIterativeSolver : public IterativeSolver
{
protected:
	const FluxFunction* const invf;
	amat::Matrix<acfd_real>* const diag;			///< Diagonal blocks
	const amat::Matrix<acfd_real>* const lambdaij;		///< Eigenvalue part of the Jacobian
	const amat::Matrix<acfd_real>* const elemflux;		///< Flux corresponding to the state at which Jacobian is to be computed
	const amat::Matrix<acfd_real>* const u;				///< state at which Jacobian is to be computed
public:
	MatrixFreeIterativeSolver(const int num_vars, const UMesh2dh* const mesh, const amat::Matrix<acfd_real>* const residual, const FluxFunction* const inviscid_flux,
			amat::Matrix<acfd_real>* const diagonal_blocks, const amat::Matrix<acfd_real>* const lambda_ij, const amat::Matrix<acfd_real>* const unk, const amat::Matrix<acfd_real>* const elem_flux)
		: IterativeSolver(num_vars, mesh, residual), invf(inviscid_flux), diag(diagonal_blocks), lambdaij(lambda_ij), u(unk), elemflux(elem_flux)
	{ }
	
	virtual void compute_update(amat::Matrix<acfd_real>* const deltau) = 0;
};

/// Matrix-free SSOR solver
/** Adapted from reference: 
 * H. Luo, D. Sharov, J.D. Baum and R. Loehner. "On the Computation of Compressible Turbulent Flows on Unstructured Grids". Internation Journal of Computational Fluid Dynamics Vol 14, No 4, pp 253-270. 2001.
 */
class SSOR_Solver : public MatrixFreeIterativeSolver
{
	
	amat::Matrix<acfd_real> f1;
	amat::Matrix<acfd_real> f2;
	amat::Matrix<acfd_real> uelpdu;
	amat::Matrix<acfd_real> elemres;
	const double w;								///< Over-relaxation factor
	acfd_int ielem, jelem, iface;
	acfd_real s, sum;
	acfd_real n[NDIM];
	int jfa;
	int ivar;
	acfd_real lambda;
	
public:

	SSOR_Solver(const int num_vars, const UMesh2dh* const mesh, const amat::Matrix<acfd_real>* const residual, const FluxFunction* const inviscid_flux,
			amat::Matrix<acfd_real>* const diagonal_blocks, const amat::Matrix<acfd_real>* const lambda_ij, const amat::Matrix<acfd_real>* const unk, const amat::Matrix<acfd_real>* const elem_flux,
			const double omega);

	/// Carries out a single step (one forward followed by one backward sweep) of SSOR and stores the correction in the argument.
	/** \note NOTE: Make sure deltau is an array of length nelem of type Matrix<acfd_real>(nvars,1).
	 */
	void compute_update(amat::Matrix<acfd_real>* const deltau);
};

// Matrix-free LU-SGS solver
/* Reference: 
 * H. Luo, D. Sharov, J.D. Baum and R. Loehner. "On the Computation of Compressible Turbulent Flows on Unstructured Grids". Internation Journal of Computational Fluid Dynamics Vol 14, No 4, pp 253-270. 2001.
 */
/*class LUSGS_Solver : public MatrixFreeIterativeSolver
{
	amat::Matrix<acfd_real>* du;
	amat::Matrix<acfd_real> f1;
	amat::Matrix<acfd_real> f2;
	amat::Matrix<acfd_real> uel;
	amat::Matrix<acfd_real> uelpdu;
	acfd_int ielem, jelem, iface;
	acfd_real s, sum;
	acfd_real n[NDIM];
	int jfa;
	int ivar;
	int ip1[NDIM], ip2[NDIM];
public:

	LUSGS_Solver(const int num_vars, const UMesh2dh* const mesh, const FluxFunction* const inviscid_flux,
			const amat::Matrix<acfd_real>* const diagonal_blocks, const amat::Matrix<acfd_real>* const residual, amat::Matrix<acfd_real>* const u);

	~LUSGS_Solver();
	
	/// Carries out a single step (one forward followed by one backward sweep) of SGS
	void update();
};*/
	
}
#endif
