#ifndef __PARTICLE_H__
#define __PARTICLE_H__



#ifndef PARTICLE
#  error : ERROR : PARTICLE is not defined !!
#endif

#ifdef GAMER_DEBUG
#  define DEBUG_PARTICLE
#endif

// Factors used in "Add(One)Particle" and "Remove(One)Particle" to resize the allocated arrays
// PARLIST_GROWTH_FACTOR must >= 1.0; PARLIST_REDUCE_FACTOR must <= 1.0
#  define PARLIST_GROWTH_FACTOR     1.1
#  define PARLIST_REDUCE_FACTOR     0.8

void Aux_Error( const char *File, const int Line, const char *Func, const char *Format, ... );




//-------------------------------------------------------------------------------------------------------
// Structure   :  Particle_t 
// Description :  Data structure of particles
//
// Data Member :  ParListSize          : Size of the particle data arrays (must be >= NPar_AcPlusInac) 
//                InactiveParListSize  : Size of the inactive particle list (InactiveParList)
//                NPar_Active_AllRank  : Total number of active particles summed up over all MPI ranks
//                NPar_AcPlusInac      : Total number of particles (active + inactive) in this MPI rank
//                NPar_Active          : Total number of active particles in this MPI rank
//                                       --> Inactive particles: particles removed from simulations because of
//                                        (1) lying outside the active region (mass == PAR_INACTIVE_OUTSIDE)
//                                        (2) being sent to other MPI ranks   (mass == PAR_INACTIVE_MPI)
//                NPar_Inactive        : Total number of inactive particles in this MPI rank
//                NPar_Lv              : Total number of active particles at each level in this MPI rank
//                Init                 : Initialization methods (1/2/3 --> call function/restart/load from file)
//                Interp               : Mass/acceleration interpolation scheme (NGP,CIC,TSC)
//                Integ                : Integration scheme (PAR_INTEG_EULER, PAR_INTEG_KDK)
//                SyncDump             : Synchronize particles in the output files (with PAR_SYNC_TEMP)
//                ImproveAcc           : Improve force accuracy around the patch boundaries
//                                       (by using potential in the patch ghost zone instead of nearby patch or interpolation)
//                PredictPos           : Predict particle position during mass assignment
//                RemoveCell           : remove particles RemoveCell-base-level-cells away from the boundary (for non-periodic BC only)
//                GhostSize            : Number of ghost zones required for interpolation scheme
//                ParVar               : Pointer arrays to different particle variables (Mass, Pos, Vel, ...)
//                Passive              : Pointer arrays to different passive variables (e.g., metalicity)
//                InactiveParList      : List of inactive particle IDs
//                Mass                 : Particle mass
//                                       Mass < 0.0 --> this particle has been removed from simulations
//                                                  --> PAR_INACTIVE_OUTSIDE: fly outside the simulation box
//                                                      PAR_INACTIVE_MPI:     sent to other MPI ranks
//                Pos                  : Particle position
//                Vel                  : Particle velocity
//                Time                 : Particle physical time
//                Acc                  : Particle acceleration (only when STORE_PAR_ACC is on)
//
// Method      :  Particle_t        : Constructor 
//               ~Particle_t        : Destructor
//                InitVar           : Initialize arrays and some variables
//                AddOneParticle    : Add one new particle into the particle list
//                RemoveOneParticle : Remove one particle from the particle list
//-------------------------------------------------------------------------------------------------------
struct Particle_t
{

// data members
// ===================================================================================
   long        ParListSize;
   long        InactiveParListSize;
   long        NPar_Active_AllRank;
   long        NPar_AcPlusInac;
   long        NPar_Active;
   long        NPar_Inactive;
   long        NPar_Lv[NLEVEL];
   ParInit_t   Init;
   ParInterp_t Interp;
   ParInteg_t  Integ;
   bool        SyncDump;
   bool        ImproveAcc;
   bool        PredictPos;
   double      RemoveCell;
   int         GhostSize;
   real       *ParVar [NPAR_VAR    ];
   real       *Passive[NPAR_PASSIVE];
   long       *InactiveParList;

   real       *Mass;
   real       *PosX;
   real       *PosY;
   real       *PosZ;
   real       *VelX;
   real       *VelY;
   real       *VelZ;
   real       *Time;
#  ifdef STORE_PAR_ACC
   real       *AccX;
   real       *AccY;
   real       *AccZ;
#  endif


   //===================================================================================
   // Constructor :  Particle_t 
   // Description :  Constructor of the structure "Particle_t"
   //
   // Note        :  Initialize the data members
   //
   // Parameter   :  None 
   //===================================================================================
   Particle_t()
   {

      NPar_Active_AllRank = -1;
      NPar_AcPlusInac     = -1;
      Init                = PAR_INIT_NONE;
      Interp              = PAR_INTERP_NONE;
      Integ               = PAR_INTEG_NONE;
      SyncDump            = true;
      ImproveAcc          = true;
      PredictPos          = true;
      RemoveCell          = -999.9;

      for (int lv=0; lv<NLEVEL; lv++)  NPar_Lv[lv] = 0;

      for (int v=0; v<NPAR_VAR;     v++)  ParVar [v] = NULL;
      for (int v=0; v<NPAR_PASSIVE; v++)  Passive[v] = NULL;

      InactiveParList = NULL;

      Mass = NULL;
      PosX = NULL;
      PosY = NULL;
      PosZ = NULL;
      VelX = NULL;
      VelY = NULL;
      VelZ = NULL;
      Time = NULL;
#     ifdef STORE_PAR_ACC 
      AccX = NULL;
      AccY = NULL;
      AccZ = NULL;
#     endif

   } // METHOD : Particle_t



   //===================================================================================
   // Destructor  :  ~Particle_t 
   // Description :  Destructor of the structure "Particle_t"
   //
   // Note        :  Free memory 
   //===================================================================================
   ~Particle_t()
   {

      for (int v=0; v<NPAR_VAR; v++)
      {
         if ( ParVar[v] != NULL )
         {
            free( ParVar[v] );
            ParVar[v] = NULL;
         }
      }

      for (int v=0; v<NPAR_PASSIVE; v++)
      {
         if ( Passive[v] != NULL )
         {
            free( Passive[v] );
            Passive[v] = NULL;
         }
      }

      if ( InactiveParList != NULL )
      {
         free( InactiveParList );
         InactiveParList = NULL;
      }

   } // METHOD : ~Particle_t



   //===================================================================================
   // Constructor :  InitVar
   // Description :  Initialize the particle attribute arrays and some other variables
   //
   // Note        :  1. NPar_AcPlusInac must be set properly (>=0) in advance 
   //                2. Initialize both "NPar_Active" and "ParListSize" as NPar_AcPlusInac
   //                3. Set "GhostSize" ("Interp" must be set in advance)
   //
   // Parameter   :  None
   //===================================================================================
   void InitVar()
   {

//    check
      if ( NPar_AcPlusInac < 0 )          Aux_Error( ERROR_INFO, "NPar_AcPlusInac (%ld) < 0 !!\n", NPar_AcPlusInac );
      if ( Interp == PAR_INTERP_NONE )    Aux_Error( ERROR_INFO, "Interp == NONE !!\n" );

//    initialize NPar_Active, NPar_Inactive, ParListSize, and InactiveParListSize
      NPar_Active         = NPar_AcPlusInac;             // assuming all particles are active initially
      NPar_Inactive       = 0;
      ParListSize         = NPar_AcPlusInac;             // set ParListSize = NPar_AcPlusInac at the beginning
      InactiveParListSize = MAX( 1, ParListSize/100 );   // set arbitrarily (but must > 0)

//    set the number of ghost zones for the interpolation scheme
      switch ( Interp )
      {
         case ( PAR_INTERP_NGP ): GhostSize = 0;   break;
         case ( PAR_INTERP_CIC ): GhostSize = 1;   break;
         case ( PAR_INTERP_TSC ): GhostSize = 1;   break;
         default: Aux_Error( ERROR_INFO, "unsupported particle interpolation scheme !!\n" );
      }

//    allocate arrays (use malloc so that realloc can be used later to resize the array)
      for (int v=0; v<NPAR_VAR;     v++)  ParVar [v] = (real*)malloc( ParListSize*sizeof(real) );
      for (int v=0; v<NPAR_PASSIVE; v++)  Passive[v] = (real*)malloc( ParListSize*sizeof(real) );

      InactiveParList = (long*)malloc( InactiveParListSize*sizeof(long) );

      Mass = ParVar[PAR_MASS];
      PosX = ParVar[PAR_POSX];
      PosY = ParVar[PAR_POSY];
      PosZ = ParVar[PAR_POSZ];
      VelX = ParVar[PAR_VELX];
      VelY = ParVar[PAR_VELY];
      VelZ = ParVar[PAR_VELZ];
      Time = ParVar[PAR_TIME];
#     ifdef STORE_PAR_ACC
      AccX = ParVar[PAR_ACCX];
      AccY = ParVar[PAR_ACCY];
      AccZ = ParVar[PAR_ACCZ];
#     endif

   } // METHOD : InitVar



   //===================================================================================
   // Method      :  AddOneParticle
   // Description :  Add ONE new particle into the particle list
   //
   // Note        :  1. This function will modify several global variables
   //                   --> One must be careful when implementing OpenMP to avoid data race
   //                   --> For example, use the **critical** construct
   //                2. Please provide all necessary information of the new particle
   //                3. If there are inactive particles, their IDs will be reassigned
   //                   to the newly added particles
   //
   // Parameter   :  NewVar      : Array storing the         variables of new particles
   //                NewPassive  : Array storing the passive variables of new particles
   //                lv          : Refinement level of the target particle
   //                AveDens     : Pointer to the global variable AveDensity
   //                _BoxVolume  : 1.0 / ( amr->BoxSize[0]*amr->BoxSize[1]*amr->BoxSize[2] )
   //===================================================================================
   void AddOneParticle( const real *NewVar, const real *NewPassive, const int lv,
                        double *AveDens, const double _BoxVolume )
   {

//    check
#     ifdef DEBUG_PARTICLE
      if ( NPar_AcPlusInac < 0 ) Aux_Error( ERROR_INFO, "NPar_AcPlusInac (%ld) < 0 !!\n", NPar_AcPlusInac );
      if ( NewVar     == NULL )  Aux_Error( ERROR_INFO, "NewVar == NULL !!\n" );
#     if ( NPAR_PASSIVE > 0 )
      if ( NewPassive == NULL )  Aux_Error( ERROR_INFO, "NewVar == NULL !!\n" );
#     endif
      if ( AveDens == NULL )     Aux_Error( ERROR_INFO, "AveDens == NULL !!\n" );
#     endif


//    1. determine the target particle ID
      long ParID;

//    1-1. reuse an inactive particle ID
      if ( NPar_Inactive > 0 )
      {
         ParID = InactiveParList[ NPar_Inactive-1 ];
         NPar_Inactive --;

#        ifdef DEBUG_PARTICLE
         if ( ParID < 0  ||  ParID >= NPar_AcPlusInac )
            Aux_Error( ERROR_INFO, "Incorrect ParID (%ld), NPar_AcPlusInac = %ld !!\n", ParID, NPar_AcPlusInac );
#        endif
      }

//    1-2. add a new particle ID
      else
      {
//       allocate enough memory for the particle variable array
         if ( NPar_AcPlusInac >= ParListSize )
         {
            ParListSize = (int)ceil( PARLIST_GROWTH_FACTOR*(ParListSize+1) );

            for (int v=0; v<NPAR_VAR;     v++)  ParVar [v] = (real*)realloc( ParVar [v], ParListSize*sizeof(real) );
            for (int v=0; v<NPAR_PASSIVE; v++)  Passive[v] = (real*)realloc( Passive[v], ParListSize*sizeof(real) );

            Mass = ParVar[PAR_MASS];
            PosX = ParVar[PAR_POSX];
            PosY = ParVar[PAR_POSY];
            PosZ = ParVar[PAR_POSZ];
            VelX = ParVar[PAR_VELX];
            VelY = ParVar[PAR_VELY];
            VelZ = ParVar[PAR_VELZ];
            Time = ParVar[PAR_TIME];
#           ifdef STORE_PAR_ACC 
            AccX = ParVar[PAR_ACCX];
            AccY = ParVar[PAR_ACCY];
            AccZ = ParVar[PAR_ACCZ];
#           endif
         }

         ParID = NPar_AcPlusInac;
         NPar_AcPlusInac ++;
      } // if ( NPar_Inactive > 0 ) ... else ...


//    2. record the data of new particles
      for (int v=0; v<NPAR_VAR;     v++)  ParVar [v][ParID] = NewVar    [v];
      for (int v=0; v<NPAR_PASSIVE; v++)  Passive[v][ParID] = NewPassive[v];

      *AveDens += Mass[ParID] * _BoxVolume;


//    3. update the active particle number (assuming all new particles are active)
      NPar_Active ++;
      NPar_Lv[lv] ++;

   } // METHOD : AddOneParticle



   //===================================================================================
   // Method      :  RemoveOneParticle
   // Description :  Remove ONE particle from the particle list
   //
   // Note        :  1. This function will modify NPar_Active, NPar_Inactive, NPar_Lv, and the input AveDens
   //                   --> Since these are global variables, one must be careful for the OpenMP
   //                       implementation to avoid data race
   //                   --> For example, use the **critical** construct
   //                2. Particles being removed will NOT be actually deleted in the memory.
   //                   Instead, their masses are assigned to Marker, which must be negative
   //                   in order to be distinguishable from active particles.
   //                   --> IDs of all removed particles will be recorded in the array InactiveParList
   //                   --> These IDs will be reassigned to new particles added later on when
   //                       calling AddOneParticle
   //
   // Parameter   :  ParID       : Particle ID to be removed
   //                Marker      : Value assigned to the mass of the particle being removed
   //                              (PAR_INACTIVE_OUTSIDE or PAR_INACTIVE_MPI)
   //                lv          : Refinement level of the target particle
   //                              --> For modifying NPar_Lv (do nothing if lv == NULL_INT)
   //                AveDens     : Pointer to the global variable AveDensity
   //                              --> Do nothing if AveDens == NULL
   //                _BoxVolume  : 1.0 / ( amr->BoxSize[0]*amr->BoxSize[1]*amr->BoxSize[2] )
   //===================================================================================
   void RemoveOneParticle( const long ParID, const real Marker, const int lv,
                           double *AveDens, const double _BoxVolume )
   {

//    check
#     ifdef DEBUG_PARTICLE
      if ( ParID < 0  ||  ParID >= NPar_AcPlusInac )
         Aux_Error( ERROR_INFO, "Wrong ParID (%ld) !!\n", ParID );

      if ( Marker != PAR_INACTIVE_OUTSIDE  &&  Marker != PAR_INACTIVE_MPI )
         Aux_Error( ERROR_INFO, "Unsupported Marker (%14.7e) !!\n", Marker );
#     endif


//    1. allocate enough memory for InactiveParList
      if ( NPar_Inactive >= InactiveParListSize )
      {
         InactiveParListSize = (int)ceil( PARLIST_GROWTH_FACTOR*(InactiveParListSize+1) );

         InactiveParList = (long*)realloc( InactiveParList, InactiveParListSize*sizeof(long) );
      }


//    2. record the particle ID to be removed
      InactiveParList[NPar_Inactive] = ParID;


//    3. remove the target particle
      if ( AveDens != NULL )  *AveDens -= Mass[ParID] * _BoxVolume;
      Mass[ParID] = Marker;

      NPar_Active --;
      if ( lv != NULL_INT )   NPar_Lv[lv] --;
      NPar_Inactive ++;

#     ifdef DEBUG_PARTICLE
      if ( NPar_Active + NPar_Inactive != NPar_AcPlusInac )
         Aux_Error( ERROR_INFO, "NPar_Active (%ld) + NPar_Inactive (%ld) != NPar_AcPlusInac (%ld) !!\n",
                    NPar_Active, NPar_Inactive, NPar_AcPlusInac );
#     endif

   } // METHOD : RemoveOneParticle


}; // struct Particle_t



#endif // #ifndef __PARTICLE_H__
