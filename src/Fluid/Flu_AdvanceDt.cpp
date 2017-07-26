#include "Copyright.h"
#include "GAMER.h"

// defined in Flu_SwapFluxPointer.cpp
void Flu_SwapFluxPointer( const int lv );
void Flu_InitTempFlux( const int lv );


extern void (*Flu_ResetByUser_API_Ptr)( const int lv, const int FluSg, const double TTime );




//-------------------------------------------------------------------------------------------------------
// Function    :  Flu_AdvanceDt
// Description :  Advance the fluid attributes by the flux gradients
//
// Note        :  a. Invoke the function "InvokeSolver"
//                b. Currently the updated data can only be stored in the different sandglass from the
//                   input fluid data
//
// Parameter   :  lv           : Target refinement level
//                TimeNew      : Target physical time to reach
//                TimeOld      : Physical time before update
//                               --> This function updates physical time from TimeOld to TimeNew
//                dt           : Time interval to advance solution (can be different from TimeNew-TimeOld in COMOVING)
//                SaveSg       : Sandglass to store the updated data
//                OverlapMPI   : true --> Overlap MPI time with CPU/GPU computation
//                Overlap_Sync : true  --> Advance the patches which cannot be overlapped with MPI communication
//                               false --> Advance the patches which can    be overlapped with MPI communication
//                               (useful only if "OverlapMPI == true")
//-------------------------------------------------------------------------------------------------------
void Flu_AdvanceDt( const int lv, const double TimeNew, const double TimeOld, const double dt, const int SaveSg,
                    const bool OverlapMPI, const bool Overlap_Sync )
{

// initialize the flux_tmp arrays for OPT__AUTO_REDUCE_DT
   if ( OPT__FIXUP_FLUX  &&  OPT__AUTO_REDUCE_DT  &&  lv != 0 )   Flu_InitTempFlux( lv-1 );


// invoke the fluid solver
   InvokeSolver( FLUID_SOLVER, lv, TimeNew, TimeOld, dt, NULL_REAL, SaveSg, NULL_INT, OverlapMPI, Overlap_Sync );


// reset the fluxes in the buffer patches at lv as zeros so that one can accumulate the coarse-fine fluxes later when evolving lv+1
   if ( OPT__FIXUP_FLUX )  Buf_ResetBufferFlux( lv );


// call Flu_ResetByUser_API_Ptr() here only if both GRAVITY and GRACKLE are disabled
#  ifdef GRAVITY
   if ( false )
#  endif
#  ifdef SUPPORT_GRACKLE
   if ( GRACKLE_MODE == GRACKLE_MODE_NONE )
#  endif
   if ( OPT__RESET_FLUID  &&  Flu_ResetByUser_API_Ptr != NULL )   Flu_ResetByUser_API_Ptr( lv, SaveSg, TimeNew );


// swap the flux pointers if the fluid solver works successfully
   if ( OPT__FIXUP_FLUX  &&  OPT__AUTO_REDUCE_DT  &&  lv != 0  &&  GAMER_SUCCESS )  Flu_SwapFluxPointer( lv-1 );

} // FUNCTION : Flu_AdvanceDt
