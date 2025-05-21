#include "TsErosion.h"
#include "TsUtility.h"

#include "CoreMinimal.h"

//#define  DDDD 1

TArray<FVector2D> TsErosion::gDebug;



struct Particle
    : public FVector2D {

    //Particle Properties
    const float		cDensity   = 1.00f; //This gives varying amounts of inertia and stuff...
    const float		cEvapRate  = 0.001f; //evaporate     0.001f
    const float		cDepoRate  = 0.30f; //deposition    0.1f
    const float		cMinVolume = 0.02f; //minimum vol   0.01f
    const float		cMaxSpeed  = 1.00f; //max speed 
    const float		cFriction  = 0.05f; //friction      0.05f

    Particle(float x, float y) : FVector2D(x, y), mSpeed(0, 0), mVolume(1.0f), mSediment(0.0f){}

    FVector2D   	mSpeed    ;
    float	    	mVolume   ; // This will vary in time
    float	    	mSediment ; // Fraction of Volume that is Sediment!
    float	    	mTime     ;

    void Simulate(  float         dt,
                    TsHeightMap*  hghtmap,
                    TsBiomeMap *  flowmap,
                    TsBiomeMap *  pondmap,
                    float         rate
        ) {
        //As long as the droplet exists...
        while (mVolume > cMinVolume) {
            int     px = X;
            int     py = Y;
            FVector n = hghtmap->CalcNormal(X, Y, 4.0f,1.0f);


            float ddt = dt;
            //Accelerate particle using newtonian mechanics using the surface normal.
            mSpeed += dt * FVector2D(n.X, n.Z) / (mVolume * cDensity);      // F = ma, so a = F/m
            float speed = mSpeed.Length();
            if (speed > cMaxSpeed) mSpeed *= cMaxSpeed / speed;

            float dv = ddt * speed;   // FMath::Max(FMath::Abs(mSpeed.X), FMath::Abs(mSpeed.Y));                // if the speed is too high, correct the time slice
            if (dv > 1.0f) ddt /= dv;

            *this  += ddt * mSpeed;                                         // Update position
            mSpeed *= (1.0 - ddt * cFriction);                              // Friction Factor

            //Check if Particle is still in-bounds
            if (!hghtmap->IsInside(px, py)) break;

            //Compute mSediment capacity difference
            float max_sediment = mVolume * mSpeed.Length() * (hghtmap->GetPixel(px, py) - hghtmap->GetPixel((int)X, (int)Y));
            if (max_sediment < 0.0) max_sediment = 0.0;
            float diff_sediment = max_sediment - mSediment;
            //Act on the Heightmap and Droplet!
            mSediment += ddt * cDepoRate * diff_sediment;

#ifdef DDDD
            UE_LOG(LogTemp, Log, TEXT("[%3.2f,%3.2f] [Speed%2.4f(%.2f,%.2f)=>(%.2f,%.2f) Vol%.2f Sed%.2f] dv%f dt%f ddt%f Flow(%f)"),
                X, Y,
                mSpeed.Length(),
                old_speed.X, old_speed.Y,
                mSpeed.X, mSpeed.Y,
                mVolume, mSediment,
                dv, dt, ddt,
                flowmap->GetPixel(px, py)
            );
#endif

            // update all biomemap
 //           flowmap->SetPixel(px, py, FMath::Min(flowmap->GetPixel(px, py) + mVolume * 0.1f, 1.0f) );
            flowmap->DrawLine(px, py, mVolume / rate, X, Y, mVolume / rate, TsImageCore::EOp::Max);
            pondmap->SetPixel(px, py, FMath::Min(pondmap->GetPixel(px, py) + ddt * 0.001f, 1.0f)); //  + mSpeed.Length() * 0.001f );
            hghtmap->SetPixel(px, py, hghtmap->GetPixel(px, py) - ddt * mVolume * cDepoRate * diff_sediment );

            //Evaporate the Droplet (Note: Proportional to Volume! Better: Use shape factor to make proportional to the area instead.)
            mVolume *= (1.0 - ddt * cEvapRate);
        }
    }
};

void TsErosion::Simulate( int cycles)
{
#define DD 4
#define DG 1.0f 

#ifdef DDDD
    Particle(192, 56).Simulate(1.2f, mHeightMap, mFlowMap, mPondMap);
#else
    for (int i = 0; i < cycles; i++) {
        // drop the rain-particle all over the world.
        for (int y = 0; y < mHeightMap->GetH(); y += DD) {
            for (int x = 0; x < mHeightMap->GetW(); x += DD) {
                auto rand = [&](int v) { return v + TsUtil::RandRange(-DG, DG) ; } ;
                Particle( rand(x), rand(y) ).
                    Simulate( 1.2f, mHeightMap, mFlowMap, mPondMap, 1.0f/ cycles ) ;
            }
        }
        UE_LOG(LogTemp, Log, TEXT("     TsErosion cycle %d/%d ... done "), i+1, cycles);
    }
#endif
}

TsErosion::TsErosion( TsHeightMap* map, TsBiomeMap* flowmap, TsBiomeMap* pondmap)
    : mHeightMap(map)
    , mFlowMap(flowmap)
    , mPondMap(pondmap)
{
}
