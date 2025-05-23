
#include "TsBiomeMap.h"



// -------------------------------- Operators --------------------------------
struct TsOp {
	union Val {
		TsOp*		op;
		float			f;
		int				i;
		TsBiomeMFunc*	task;

		Val(float         _v) : f   (_v) {}
		Val(int           _v) : i   (_v) {}
		Val(TsOp*    _v) : op  (_v) {}
		Val(TsBiomeMFunc* _v) : task(_v) {}
	};
	TArray<Val>				mArgs;

	TsOp(TArray<Val> args) : mArgs(args) {}
	virtual ~TsOp() {}

	virtual bool Is(const FVector2D& p) const { return true; }	//tex-coord

	static TsMaterialPixel	gMatResult;
	static bool				gResultDone;
};

#ifdef	DECLARE
TsMaterialPixel	TsOp::gMatResult = TsMaterialPixel();
bool			TsOp::gResultDone = false;
#endif	// DECLARE



struct TsOp_Or : public TsOp {
	TsOp_Or(TsOp* a, TsOp* b) : TsOp({ Val(a), Val(b) }) {}
	virtual ~TsOp_Or() {}
	bool Is(const FVector2D& p) const override {
		return mArgs[0].op->Is(p) || mArgs[1].op->Is(p);
	}
};

struct TsOp_And : public TsOp {
	TsOp_And(TsOp* a, TsOp* b) : TsOp({ Val(a), Val(b) }) {}
	virtual ~TsOp_And() {}
	bool Is(const FVector2D& p) const override {
		return mArgs[0].op->Is(p) && mArgs[1].op->Is(p);
	}
};

struct TsOp_Not : public TsOp {
	TsOp_Not(TsOp* a) : TsOp({ Val(a) }) {}
	virtual ~TsOp_Not() {}
	bool Is(const FVector2D& p) const override {
		return !mArgs[0].op->Is(p);
	}
};

