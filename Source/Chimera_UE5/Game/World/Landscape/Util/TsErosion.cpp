#include "TsErosion.h"
#include "TsUtility.h"

#include "CoreMinimal.h"


TArray<FVector2D> TsErosion::gDebug;



struct Particle
    : public FVector2D {
    Particle(float x, float y) : FVector2D(x, y), mSpeed(0, 0), mVolume(1.0f), mSediment(0.0f){}

    FVector2D   	mSpeed    ;
    float	    	mVolume   ; // This will vary in time
    float	    	mSediment ; // Fraction of Volume that is Sediment!

    //Particle Properties
    const float		cDensity    = 1.0f  ;   //This gives varying amounts of inertia and stuff...
    const float		cEvapRate   = 0.01f;   //evaporate     0.001f
    const float		cDepoRate   = 0.1f  ;   //deposition    0.1f
    const float		cMinVolume  = 0.01f ;   //minimum vol   0.01f
    const float		cFriction   = 0.05f ;   //friction      0.05f

    void Simulate(  float       dt,
                    TsHeightMap*  hghtmap,
                    TsBiomeMap *  flowmap,
                    TsBiomeMap*   pondmap ) {
        int     qx = X, qy = Y;
        float   qv = 0;

        //As long as the droplet exists...
        while (mVolume > cMinVolume) {
            int     px = X;
            int     py = Y;
            FVector n = hghtmap->CalcNormal(X, Y, 1.0f, 1.0f);

            float ddt = dt;
            //Accelerate particle using newtonian mechanics using the surface normal.
            mSpeed += dt * FVector2D(n.X, n.Z) / (mVolume * cDensity);      // F = ma, so a = F/m
            float dv = ddt * FMath::Max(FMath::Abs(mSpeed.X), FMath::Abs(mSpeed.Y));                // if the speed is too high, correct the time slice
            if (dv > 1.0f) ddt /= dv;

            //UE_LOG(LogTemp, Log, TEXT(" SP[%f,%f] dv%f dt%f ddt%f"), mSpeed.X, mSpeed.Y, dv, dt, ddt );
            *this += ddt * mSpeed;
            mSpeed *= (1.0 - ddt * cFriction);                              // Friction Factor
            //UE_LOG(LogTemp, Log, TEXT(" =>[%f,%f](%f %f) "), mSpeed.X, mSpeed.Y, X, Y);

            //Check if Particle is still in-bounds
            if (!hghtmap->IsInside(px, py)) break;

            //Compute mSediment capacity difference
            float max_sediment = mVolume * mSpeed.Length() * (hghtmap->GetPixel(px, py) - hghtmap->GetPixel((int)X, (int)Y));
            if (max_sediment < 0.0) max_sediment = 0.0;
            float diff_sediment = max_sediment - mSediment;
            //Act on the Heightmap and Droplet!
            mSediment += ddt * cDepoRate * diff_sediment;

            // update all biomemap
//            if ( px!=qx && py!=qy )
//            flowmap->SetPixel(px, py, FMath::Max(flowmap->GetPixel(px, py), mSpeed.Length() ));
            //flowmap->SetPixel(px, py, FMath::Max(flowmap->GetPixel(px, py), dt * mVolume * cDepoRate * diff_sediment * 1000000.0f));
            flowmap->SetPixel(px, py, flowmap->GetPixel(px, py) + ddt * mVolume * cDepoRate * diff_sediment * 10.0f );
            //flowmap->DrawLine( qx, qy, qv,   px, py, mSpeed.Length(),  ImageCore::Op::Set );

            pondmap->SetPixel(px, py, FMath::Min(pondmap->GetPixel(px, py) + ddt * 0.001f, 2.0f)); //  + mSpeed.Length() * 0.001f );
            hghtmap->SetPixel(px, py, hghtmap->GetPixel(px, py) - ddt * mVolume * cDepoRate * diff_sediment);

            //Evaporate the Droplet (Note: Proportional to Volume! Better: Use shape factor to make proportional to the area instead.)
            mVolume *= (1.0 - ddt * cEvapRate);
            qv = mSpeed.Length();
            qx = px, qy = py;
        }
    }
};

void TsErosion::Simulate( int cycles)
{
#define DD 4
#define DG 1.0f 
    for (int i = 0; i < cycles; i++) {
        // drop the rain-particle all over the world.
        for (int y = 0; y < mHeightMap->GetH(); y += DD) {
            for (int x = 0; x < mHeightMap->GetW(); x += DD) {
                auto rand = [&](int v) { return v + TsUtil::RandRange(-DG, DG) ; } ;
                Particle( rand(x), rand(y) ).
                    Simulate( 1.2f, mHeightMap, mFlowMap, mPondMap) ;
            }
        }
        UE_LOG(LogTemp, Log, TEXT("     TsErosion cycle %d/%d ... done "), i+1, cycles);
    }
}

TsErosion::TsErosion( TsHeightMap* map, TsBiomeMap* flowmap, TsBiomeMap* pondmap)
    : mHeightMap(map)
    , mFlowMap(flowmap)
    , mPondMap(pondmap)
{
}
