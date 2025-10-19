#pragma once
#include <cstdint>
#include <atomic>
#include <mutex>
#include <shared_mutex>

class Metric{
public:
	struct MetricSlot{
		std::atomic_uint64_t totalDelayTime = 0;//µÙ∑π¿Ã √—«’
		std::atomic_uint32_t totalDelayCnt = 0;//µÙ∑π¿Ãø° √ﬂ∞°µ» «•∫ª ∞πºˆ
		MetricSlot() = default;

		void Clear(){
			totalDelayTime = 0;
			totalDelayCnt = 0;
		}
	};

	static Metric & GetInstance(){
		static Metric metric;
		return metric;
	}
private:
	Metric() = default;
public:
	//ΩÃ±€≈Ê¿Ã±‚ ãöπÆø°, ∫πªÁ ¿Ãµø xx
	Metric(const Metric &) = delete;
	Metric(Metric &&) = delete;

	MetricSlot & SwapAndLoad(){
		int prev = 0;
		prev = currentIdx;

		int otherIdx = ( prev + 1 ) % 2;
		m_metrics[otherIdx].Clear();
		currentIdx = otherIdx;

		return m_metrics[prev];
	}

	MetricSlot & GetCurrentMetric(){
		return m_metrics[currentIdx.load()];
	}

private:
	MetricSlot m_metrics[2];
	std::atomic_uint currentIdx = 0;
};
