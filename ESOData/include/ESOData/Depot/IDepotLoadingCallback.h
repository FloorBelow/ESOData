#ifndef ESODATA_DEPOT_I_DEPOT_LOADING_CALLBACK_H
#define ESODATA_DEPOT_I_DEPOT_LOADING_CALLBACK_H

namespace esodata {
	class IDepotLoadingCallback {
	protected:
		inline IDepotLoadingCallback() = default;
		inline ~IDepotLoadingCallback() = default;

	public:
		virtual bool loadingStepsDone(unsigned int steps) = 0;
	};
}

#endif
