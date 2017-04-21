#ifndef __MACRO_H__
#define __MACRO_H__



// ****************************************************************************
// ** This header defines the symbolic constants and macros used in GAMER.   **
// ** For clarity, useless options defined in the makefile will be "undef"   **
// ** in the end of this file.                                               **
// ****************************************************************************


// ########################
// ## Symbolic Constants ##
// ########################

// option == NONE --> the option is turned off
#define NONE         0


// GPU architecture
#define FERMI        1
#define KEPLER       2
#define MAXWELL      3
#define PASCAL       4


// models
#define HYDRO        1
#define MHD          2
#define ELBDM        3
#define PAR_ONLY     4


// hydrodynamic schemes
#define RTVD         1
#define WAF          2
#define MHM          3
#define MHM_RP       4
#define CTU          5


// data reconstruction schemes
#define PLM          1
#define PPM          2


// Riemann solvers
#define EXACT        1
#define ROE          2
#define HLLE         3
#define HLLC         4


// dual-energy variables
#define DE_ENPY      1
#define DE_EINT      2

#ifdef DUAL_ENERGY
#define DE_UPDATED_BY_ETOT       ('0')
#define DE_UPDATED_BY_DUAL       ('1')
#define DE_UPDATED_BY_MIN_PRES   ('2')
#define DE_UPDATED_BY_1ST_FLUX   ('3')
#define DE_UPDATED_BY_ETOT_GRA   ('4')
#endif


// Poisson solvers
#define SOR          1
#define MG           2


// load-balance parallelization
#define HILBERT      1


// NCOMP_FLUID : number of active components in each cell (i.e., the "fluid" array)
//               --> do not include passive components here, which is set by NCOMP_PASSIVE
// NFLUX_FLUID : number of active components in the "flux" array
//               --> do not include passive components here, which is set by NFLUX_PASSIVE
#if   ( MODEL == HYDRO )
#  define NCOMP_FLUID         5
#  define NFLUX_FLUID         NCOMP_FLUID

#elif ( MODEL == MHD )
#  warning : WAIT MHD !!!
#  define NCOMP_FLUID         5
#  define NFLUX_FLUID         NCOMP_FLUID

// for ELBDM, we only need the density flux
#elif ( MODEL == ELBDM )
#  define NCOMP_FLUID         3
#  define NFLUX_FLUID         1

#elif ( MODEL == PAR_ONLY )
#  define NCOMP_FLUID         0
#  define NFLUX_FLUID         0

#else
#  error : ERROR : unsupported MODEL (please edit NCOMP_FLUID and NFLUX_FLUID for the new MODEL) !!
#endif // MODEL


// number of passively advected components in each cell
// --> including entropy (or internal energy) when the dual energy formalism is adopted
#if (  ( MODEL == HYDRO || MODEL == MHD )  &&  defined DUAL_ENERGY  )
#  define NCOMP_PASSIVE       ( NCOMP_PASSIVE_MAKEFILE + 1 )
#else
#  define NCOMP_PASSIVE       ( NCOMP_PASSIVE_MAKEFILE )
#endif

// assuming all passive scalars have the corresponding fluxes
#  define NFLUX_PASSIVE       NCOMP_PASSIVE


// total number of variables in each cell and in the flux array including both active and passive variables
#  define NCOMP_TOTAL         ( NCOMP_FLUID + NCOMP_PASSIVE )
#  define NFLUX_TOTAL         ( NFLUX_FLUID + NFLUX_PASSIVE )


// number of input/output variables in the fluid solver
#if   ( MODEL == HYDRO )
#  define FLU_NIN             NCOMP_TOTAL
#  define FLU_NOUT            NCOMP_TOTAL

#elif ( MODEL == MHD )
#  warning : WAIT MHD !!!
#  define FLU_NIN             NCOMP_TOTAL
#  define FLU_NOUT            NCOMP_TOTAL

// for ELBDM, we do not need to transfer the density component into GPU
#elif ( MODEL == ELBDM )
#  define FLU_NIN             ( NCOMP_TOTAL-1 )
#  define FLU_NOUT            ( NCOMP_TOTAL-0 )

#elif ( MODEL == PAR_ONLY )
#  define FLU_NIN             0
#  define FLU_NOUT            0

#else
#  error : ERROR : unsupported MODEL (please edit FLU_NIN and FLU_NOUT for the new MODEL) !!
#endif // MODEL


// main variables in different models
// --> note that we must set "_VAR_NAME = 1<<VAR_NAME" (e.g., _DENS == 1<<DENS)
#if   ( MODEL == HYDRO )

// variable indices in the array "fluid" [0 ... NCOMP_FLUID-1]
#  define  DENS               0
#  define  MOMX               1
#  define  MOMY               2
#  define  MOMZ               3
#  define  ENGY               4

// variable indices in the array "passive" [NCOMP_FLUID ... NCOMP_TOTAL-1]
#if ( NCOMP_PASSIVE > 0 )
// example for NCOMP_PASSIVE == 3
/*
#  define  METAL              ( NCOMP_FLUID + 0 )
#  define  OXYGEN             ( NCOMP_FLUID + 1 )
#  define  FE                 ( NCOMP_FLUID + 2 )
*/
// always store entropy (or internal energy) for the dual energy formalism as the last passive variable
#  if   ( DUAL_ENERGY == DE_ENPY )
#  define  ENPY               ( NCOMP_TOTAL-1 )
#  elif ( DUAL_ENERGY == DE_EINT )
#  define  EINT               ( NCOMP_TOTAL-1 )
#  endif
#endif

// variable indices in the array "flux" [0 ... NFLUX_FLUID-1]
#  define  FLUX_DENS          0
#  define  FLUX_MOMX          1
#  define  FLUX_MOMY          2
#  define  FLUX_MOMZ          3
#  define  FLUX_ENGY          4

// variable indices in the array "flux_passive" [NFLUX_FLUID ... NFLUX_TOTAL-1]
#if ( NCOMP_PASSIVE > 0 )
// example for NCOMP_PASSIVE == 3
/*
#  define  FLUX_METAL         ( NFLUX_FLUID + 0 )
#  define  FLUX_OXYGEN        ( NFLUX_FLUID + 1 )
#  define  FLUX_FE            ( NFLUX_FLUID + 2 )
*/
// always store entropy (or internal energy) for the dual energy formalism as the last passive variable
#  if   ( DUAL_ENERGY == DE_ENPY )
#  define  FLUX_ENPY          ( NFLUX_TOTAL-1 )
#  elif ( DUAL_ENERGY == DE_EINT )
#  define  FLUX_EINT          ( NFLUX_TOTAL-1 )
#  endif
#endif

// symbolic constants used as function parameters (e.g., Prepare_PatchData)
#  define _DENS               ( 1 << DENS )
#  define _MOMX               ( 1 << MOMX )
#  define _MOMY               ( 1 << MOMY )
#  define _MOMZ               ( 1 << MOMZ )
#  define _ENGY               ( 1 << ENGY )

#if ( NCOMP_PASSIVE > 0 )
// example for NCOMP_PASSIVE == 3
/*
#  define _METAL              ( 1 << METAL  )
#  define _OXYGEN             ( 1 << OXYGEN )
#  define _FE                 ( 1 << FE     )
*/
// always store entropy (or internal energy) for the dual energy formalism as the last passive variable
#  if   ( DUAL_ENERGY == DE_ENPY )
#  define _ENPY               ( 1 << ENPY )
#  elif ( DUAL_ENERGY == DE_EINT )
#  define _EINT               ( 1 << EINT )
#  endif
#endif // #if ( NCOMP_PASSIVE > 0 )

// symbolic constants of flux used as function parameters (e.g., Buf_GetBufferData)
#  define _FLUX_DENS          ( 1 << FLUX_DENS )
#  define _FLUX_MOMX          ( 1 << FLUX_MOMX )
#  define _FLUX_MOMY          ( 1 << FLUX_MOMY )
#  define _FLUX_MOMZ          ( 1 << FLUX_MOMZ )
#  define _FLUX_ENGY          ( 1 << FLUX_ENGY )

#if ( NFLUX_PASSIVE > 0 )
// example for NFLUX_PASSIVE == 3
/*
#  define _FLUX_METAL         ( 1 << FLUX_METAL  )
#  define _FLUX_OXYGEN        ( 1 << FLUX_OXYGEN )
#  define _FLUX_FE            ( 1 << FLUX_FE     )
*/
// always store entropy (or internal energy) for the dual energy formalism as the last passive variable
#  if   ( DUAL_ENERGY == DE_ENPY )
#  define _FLUX_ENPY          ( 1 << FLUX_ENPY )
#  elif ( DUAL_ENERGY == DE_EINT )
#  define _FLUX_EINT          ( 1 << FLUX_EINT )
#  endif
#endif // #if ( NFLUX_PASSIVE > 0 )

// derived variables
// note that _POTE = ( 1 << NCOMP_TOTAL )
#  define _VELX               ( 1 << (NCOMP_TOTAL+1) )
#  define _VELY               ( 1 << (NCOMP_TOTAL+2) )
#  define _VELZ               ( 1 << (NCOMP_TOTAL+3) )
#  define _PRES               ( 1 << (NCOMP_TOTAL+4) )
#  define _TEMP               ( 1 << (NCOMP_TOTAL+5) )
#  define _DERIVED            ( _VELX | _VELY | _VELZ | _PRES | _TEMP )


#elif ( MODEL == MHD )
#  warning : WAIT MHD !!!


#elif ( MODEL == ELBDM )
// variable indices in the array "fluid"
#  define  DENS               0
#  define  REAL               1
#  define  IMAG               2

// variable indices in the array "flux" [0 ... NFLUX_FLUID-1]
#  define  FLUX_DENS          0

// symbolic constants used as function parameters (e.g., Prepare_PatchData)
#  define _DENS               ( 1 << DENS )
#  define _REAL               ( 1 << REAL )
#  define _IMAG               ( 1 << IMAG )

// symbolic constants of flux used as function parameters (e.g., Buf_GetBufferData)
#  define _FLUX_DENS          ( 1 << FLUX_DENS )

// derived variables
#  define _DERIVED            0


#elif ( MODEL == PAR_ONLY )
#  define _DERIVED            0


#else
#  error : ERROR : unsupported MODEL !!
#endif // MODEL


// symbolic constants used by all models
#  define _FLUID              (  ( 1 << NCOMP_FLUID ) - 1           )
#  define _PASSIVE            (  ( 1 << NCOMP_TOTAL ) - 1 - _FLUID  )
#  define _TOTAL              (  ( 1 << NCOMP_TOTAL ) - 1           )
#  ifdef GRAVITY
#  define _POTE               (  ( 1 << NCOMP_TOTAL )               )
#  endif

#  define _FLUX_FLUID         (  ( 1 << NFLUX_FLUID ) - 1                )
#  define _FLUX_PASSIVE       (  ( 1 << NFLUX_TOTAL ) - 1 - _FLUX_FLUID  )
#  define _FLUX_TOTAL         (  ( 1 << NFLUX_TOTAL ) - 1                )


// symbolic constants for particles
#ifdef PARTICLE

// number of variables stored in each particle (excluding the passive variables)
#  ifdef STORE_PAR_ACC
#  define PAR_NVAR            ( 11 + 0 )
#  else
#  define PAR_NVAR            (  8 + 0 )
#  endif

// variable indices in the array "ParVar" [0 ... PAR_NVAR-1]
#  define  PAR_MASS           0
#  define  PAR_POSX           1
#  define  PAR_POSY           2
#  define  PAR_POSZ           3
#  define  PAR_VELX           4
#  define  PAR_VELY           5
#  define  PAR_VELZ           6
#  define  PAR_TIME           7
#  define  PAR_ACCX           8
#  define  PAR_ACCY           9
#  define  PAR_ACCZ          10

// passive variable indices in the array "Passive" [0 ... PAR_NPASSIVE-1]
// --> note that unlike the passive scalars on cells, the indices of passive particle attributes start from 0
// --> this may be modified in the future
#if ( PAR_NPASSIVE > 0 )
// example for PAR_NPASSIVE == 3
/*
#  define  PAR_METAL          0
#  define  PAR_OXYGEN         1
#  define  PAR_FE             2
*/
#endif

// symbolic constants used as function parameters (e.g., Prepare_PatchData)
#  if ( MODEL == PAR_ONLY )
// note that _POTE == ( 1 << 0 )
#  define _PAR_DENS           ( 1 << 1 )
#  define _TOTAL_DENS         ( _PAR_DENS )

#  else

// note that _TEMP == ( 1 << (NCOMP_TOTAL+5) )
#  define _PAR_DENS           ( 1 << (NCOMP_TOTAL+6) )
#  define _TOTAL_DENS         ( 1 << (NCOMP_TOTAL+7) )

#  endif // if ( MODEL == PAR_ONLY ) ... else ...

#else // #ifdef PARTICLE

// set _TOTAL_DENS == _DENS if PARTICLE is off
#  define _TOTAL_DENS         ( _DENS )

#endif // #ifdef PARTICLE ... else ...


// number of fluid ghost zones for the fluid solver
#if   ( MODEL == HYDRO )   // hydro
#  if   ( FLU_SCHEME == RTVD )
#        define FLU_GHOST_SIZE      3
#  elif ( FLU_SCHEME == WAF )
#        define FLU_GHOST_SIZE      2
#  elif ( FLU_SCHEME == MHM )
#     if ( LR_SCHEME == PLM )
#        define FLU_GHOST_SIZE      2
#     else // PPM
#        define FLU_GHOST_SIZE      3
#     endif
#  elif ( FLU_SCHEME == MHM_RP )
#     if ( LR_SCHEME == PLM )
#        define FLU_GHOST_SIZE      3
#     else // PPM
#        define FLU_GHOST_SIZE      4
#     endif
#  elif ( FLU_SCHEME == CTU )
#     if ( LR_SCHEME == PLM )
#        define FLU_GHOST_SIZE      2
#     else // PPM
#        define FLU_GHOST_SIZE      3
#     endif
#  endif

#elif ( MODEL == MHD )     // MHD
#        warning : WAIT MHD !!!
#        define FLU_GHOST_SIZE      ?

#elif ( MODEL == ELBDM )   // ELBDM
#  ifdef LAPLACIAN_4TH
#        define FLU_GHOST_SIZE      6
#  else
#        define FLU_GHOST_SIZE      3
#  endif

#else
#  error : ERROR : unsupported MODEL !!
#endif // MODEL


// self-gravity constants
#ifdef GRAVITY

// number of input and output variables in the gravity solver
#  if   ( MODEL == HYDRO )
#     define GRA_NIN             NCOMP_FLUID

#  elif ( MODEL == MHD )
#     warning : WAIT MHD !!!
#     define GRA_NIN             NCOMP_FLUID

// for ELBDM, we do not need to transfer the density component
#  elif ( MODEL == ELBDM )
#     define GRA_NIN             ( NCOMP_FLUID - 1 )

#  else
#     error Error : unsupported MODEL (please edit GRA_NIN in the new MODEL) !!
#  endif // MODEL


// number of potential ghost zones for evaluating potential (maximum=5) ~ Poisson solver
#     define POT_GHOST_SIZE      5


// number of potential ghost zones for advancing fluid by gravity ~ Gravity solver
#  if   ( MODEL == HYDRO )
#     ifdef STORE_POT_GHOST
#     define GRA_GHOST_SIZE      2
#     else
#     define GRA_GHOST_SIZE      1
//#   define GRA_GHOST_SIZE      2
#     endif

#  elif ( MODEL == MHD )
#     ifdef STORE_POT_GHOST
#     define GRA_GHOST_SIZE      2
#     else
#     define GRA_GHOST_SIZE      1
//#   define GRA_GHOST_SIZE      2
#     endif

#  elif ( MODEL == ELBDM )
#     ifdef STORE_POT_GHOST
#     define GRA_GHOST_SIZE      2
#     else
#     define GRA_GHOST_SIZE      0
#     endif

#  elif ( MODEL == PAR_ONLY )
#     ifdef STORE_POT_GHOST
#     define GRA_GHOST_SIZE      2
#     else
#     define GRA_GHOST_SIZE      0
#     endif

#  else
#     error : ERROR : unsupported MODEL !!
#  endif // MODEL


// number of potential ghost zones for correcting the half-step velocity if UNSPLIT_GRAVITY is on
#  ifdef UNSPLIT_GRAVITY
#  if   ( MODEL == HYDRO )
#     define USG_GHOST_SIZE      1
#  elif ( MODEL == MHD )
#     define USG_GHOST_SIZE      1
#  elif ( MODEL == ELBDM )
#     define USG_GHOST_SIZE      0
#  else
#     error : ERROR : unsupported MODEL !!
#  endif // MODEL
#  endif // #ifdef UNSPLIT_GRAVITY


// number of density ghost zones for storing the temporary particle mass density (in the array rho_ext)
#  ifdef PARTICLE
#     define RHOEXT_GHOST_SIZE  2
#  endif


// number of density ghost zones for the Poisson solver
#     define RHO_GHOST_SIZE      ( POT_GHOST_SIZE-1 )

#endif // #ifdef GRAVITY


// patch size (number of cells of a single patch in the x/y/z directions)
#define PATCH_SIZE                   8
#define PS1             ( 1*PATCH_SIZE )
#define PS2             ( 2*PATCH_SIZE )


// the size of arrays (in one dimension) sending into GPU
//###REVISE: support interpolation schemes requiring 2 ghost cells on each side for POT_NXT
#  define FLU_NXT       ( 2*(PATCH_SIZE+FLU_GHOST_SIZE)   )             // use patch group as the unit
#ifdef GRAVITY
#  define POT_NXT       ( PATCH_SIZE/2 + 2*( (POT_GHOST_SIZE+3)/2 ) )   // assuming interpolation ghost zone == 1
#  define RHO_NXT       ( PATCH_SIZE   + 2*RHO_GHOST_SIZE )             // POT/RHO/GRA_NXT use patch as the unit
#  define GRA_NXT       ( PATCH_SIZE   + 2*GRA_GHOST_SIZE )
#  ifdef UNSPLIT_GRAVITY
#  define USG_NXT_F     ( 2*(PATCH_SIZE+USG_GHOST_SIZE)   )             // we use patch group as unit for the fluid   solver
#  define USG_NXT_G     ( PATCH_SIZE   + 2*USG_GHOST_SIZE )             // we use patch       as unit for the gravity solver
#  else
#  define USG_NXT_F     ( 1 )                                           // still define USG_NXT_F/G since many function prototypes
#  define USG_NXT_G     ( 1 )                                           // require it
#  endif
#else
#  define USG_NXT_F     ( 1 )                                           // still define USG_NXT_F ...
#endif
#ifdef PARTICLE
#  define RHOEXT_NXT    ( PATCH_SIZE   + 2*RHOEXT_GHOST_SIZE )          // array rho_ext of each patch
#endif


// extreme values
#ifndef __INT_MAX__
#  define __INT_MAX__      2147483647
#endif

#ifndef __LONG_MAX__
#  define __LONG_MAX__     9223372036854775807L
#endif

#ifndef __UINT_MAX__
#  define __UINT_MAX__     ( __INT_MAX__*2U + 1U )
#endif

#ifndef __ULONG_MAX__
#  define __ULONG_MAX__    ( 18446744073709551615UL )    // 2^64-1
#endif

#ifndef __FLT_MAX__
#  define __FLT_MAX__      3.40282347e+38F
#endif

#ifndef __FLT_MIN__
#  define __FLT_MIN__      1.17549435e-38F
#endif


// extreme value used for various purpose (e.g., floor value for passive scalars)
#define TINY_NUMBER        __FLT_MIN__
#define HUGE_NUMBER        __FLT_MAX__


// sibling index offset for the non-periodic B.C.
#define SIB_OFFSET_NONPERIODIC   ( -100 )


// son index offset for LOAD_BALANCE
#ifdef LOAD_BALANCE
#  define SON_OFFSET_LB          ( -1000 )
#endif


// flag used in "Buf_RecordBoundaryFlag" and "Flag_Buffer" (must be negative)
#ifndef SERIAL
#  define BUFFER_IS_FLAGGED      ( -999 )
#endif


// marker indicating that the array "pot_ext" has NOT been properly set
#if ( defined GRAVITY  &&  defined STORE_POT_GHOST )
#  define POT_EXT_NEED_INIT      __FLT_MAX__
#endif


// marker indicating that the array "rho_ext" has NOT been properly set
#ifdef PARTICLE
#  define RHO_EXT_NEED_INIT      __FLT_MAX__
#endif


// markers for inactive particles
#ifdef PARTICLE
#  define PAR_INACTIVE_OUTSIDE   ( -1.0 )
#  define PAR_INACTIVE_MPI       ( -2.0 )
#endif


// NULL values
#ifndef NULL
#  define NULL             0
#endif

#ifndef NULL_INT
#  define NULL_INT         __INT_MAX__
#endif

#ifndef NULL_REAL
#  define NULL_REAL        __FLT_MAX__
#endif

#ifndef NULL_BOOL
#  define NULL_BOOL        false
#endif


// GAMER status
#define GAMER_SUCCESS      1
#define GAMER_FAILED       0


// macro for the function "Aux_Error"
#define ERROR_INFO         __FILE__, __LINE__, __FUNCTION__


// miscellaneous
#define TOP_LEVEL          ( NLEVEL - 1 )


// maximum length for strings
#define MAX_STRING         512



// ############
// ## Macros ##
// ############

// single/double-precision mathematic functions
#ifdef FLOAT8
#  define  FABS( a )        fabs( a )
#  define  SQRT( a )        sqrt( a )
#  define   SIN( a )         sin( a )
#  define   COS( a )         cos( a )
#  define   LOG( a )         log( a )
#  define   EXP( a )         exp( a )
#  define  ATAN( a )        atan( a )
#  define FLOOR( a )       floor( a )
#  define  FMAX( a, b )     fmax( a, b )
#  define  FMIN( a, b )     fmin( a, b )
#  define   POW( a, b )      pow( a, b )
#  define  FMOD( a, b )     fmod( a, b )
#  define ATAN2( a, b )    atan2( a, b )
#else
#  define  FABS( a )        fabsf( a )
#  define  SQRT( a )        sqrtf( a )
#  define   SIN( a )         sinf( a )
#  define   COS( a )         cosf( a )
#  define   LOG( a )         logf( a )
#  define   EXP( a )         expf( a )
#  define  ATAN( a )        atanf( a )
#  define FLOOR( a )       floorf( a )
#  define  FMAX( a, b )     fmaxf( a, b )
#  define  FMIN( a, b )     fminf( a, b )
#  define   POW( a, b )      powf( a, b )
#  define  FMOD( a, b )     fmodf( a, b )
#  define ATAN2( a, b )    atan2f( a, b )
#endif


// sign function
#define SIGN( a )       (  ( (a) < (real)0.0 ) ? (real)-1.0 : (real)+1.0  )


// max/min functions
#define MAX( a, b )     (  ( (a) > (b) ) ? (a) : (b)  )
#define MIN( a, b )     (  ( (a) < (b) ) ? (a) : (b)  )


// square/cube function
#define SQR(  a )       ( (a)*(a)     )
#define CUBE( a )       ( (a)*(a)*(a) )


// 3D to 1D array indices transformation
#define IDX321( i, j, k, Ni, Nj )   (  ( (k)*(Nj) + (j) )*(Ni) + (i)  )



// ################################
// ## Remove useless definitions ##
// ################################
#if ( MODEL == HYDRO )
#  if ( FLU_SCHEME != MHM  &&  FLU_SCHEME != MHM_RP  &&  FLU_SCHEME != CTU )
#  undef LR_SCHEME
#  endif

#  if ( FLU_SCHEME != MHM  &&  FLU_SCHEME != MHM_RP  &&  FLU_SCHEME != CTU  &&  FLU_SCHEME != WAF )
#  undef RSOLVER
#  endif

#elif ( MODEL == MHD )
#  warning : WAIT MHD !!!
#endif // MODEL

#if ( MODEL != HYDRO  &&  MODEL != MHD )
#  undef FLU_SCHEME
#  undef LR_SCHEME
#  undef RSOLVER
#  undef DUAL_ENERGY
#endif

#ifndef GRAVITY
#  undef POT_SCHEME
#  undef STORE_POT_GHOST
#  undef UNSPLIT_GRAVITY
#endif

#if ( MODEL == PAR_ONLY )
#  undef UNSPLIT_GRAVITY
#endif

#ifndef GPU
#  undef GPU_ARCH
#endif



#endif  // #ifndef __MACRO_H__
