/*!
 * UFOMap: An Efficient Probabilistic 3D Mapping Framework That Embraces the Unknown
 *
 * @author Daniel Duberg (dduberg@kth.se)
 * @see https://github.com/UnknownFreeOccupied/ufomap
 * @version 1.0
 * @date 2022-05-13
 *
 * @copyright Copyright (c) 2022, Daniel Duberg, KTH Royal Institute of Technology
 *
 * BSD 3-Clause License
 *
 * Copyright (c) 2022, Daniel Duberg, KTH Royal Institute of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef UFO_PCL_POINT_CLOUD_HPP
#define UFO_PCL_POINT_CLOUD_HPP

// UFO
// TODO: Add dependency on ufomath
#include <ufo/math/transform.hpp>
#include <ufo/math/vec.hpp>
#include <ufo/pcl/cloud.hpp>
// TODO: Add dependency on ufoutility
#include <ufo/utility/execution.hpp>

// STL
#include <cstddef>
#include <limits>

namespace ufo
{
template <std::size_t Dim, class T, class... Rest>
using PointCloud = Cloud<Vec<Dim, T>, Rest...>;

//
// Transform
//

template <std::size_t Dim, class T, class... Rest>
[[nodiscard]] PointCloud<Dim, T, Rest...> transform(Transform<Dim, T> const&    t,
                                                    PointCloud<Dim, T, Rest...> pc)
{
	return transformInPlace(execution::seq, t, pc);
}

template <
    class ExecutionPolicy, std::size_t Dim, class T, class... Rest,
    std::enable_if_t<execution::is_execution_policy_v<ExecutionPolicy>, bool> = true>
[[nodiscard]] PointCloud<Dim, T, Rest...> transform(ExecutionPolicy&&           policy,
                                                    Transform<Dim, T> const&    t,
                                                    PointCloud<Dim, T, Rest...> pc)
{
	transformInPlace(std::forward<ExecutionPolicy>(policy), t, pc);
	return pc;
}

template <std::size_t Dim, class T, class... Rest>
void transformInPlace(Transform<Dim, T> const& t, PointCloud<Dim, T, Rest...>& pc)
{
	transformInPlace(execution::seq, t, pc);
}

template <
    class ExecutionPolicy, std::size_t Dim, class T, class... Rest,
    std::enable_if_t<execution::is_execution_policy_v<ExecutionPolicy>, bool> = true>
void transformInPlace(ExecutionPolicy&& policy, Transform<Dim, T> const& t,
                      PointCloud<Dim, T, Rest...>& pc)
{
	transform(std::forward<ExecutionPolicy>(policy), t, get<0>(pc));
}

//
// Filter
//

template <
    class ExecutionPolicy, std::size_t Dim, class T, class... Rest,
    std::enable_if_t<execution::is_execution_policy_v<ExecutionPolicy>, bool> = true>
void filterDistanceInPlace(ExecutionPolicy&& policy, PointCloud<Dim, T, Rest...>& pc,
                           Vec<Dim, T> const& origin, T const& min_distance,
                           T const& max_distance)
{
	if (T(0) >= min_distance && std::numeric_limits<T>::max() <= max_distance) {
		return;
	}

	auto const min_sq = min_distance * min_distance;
	auto const max_sq = max_distance * max_distance;

	if constexpr (execution::is_seq_v<ExecutionPolicy>) {
		auto it =
		    std::remove_if(pc.begin(), pc.end(), [&origin, &min_sq, &max_sq](Vec<Dim, T> x) {
			    auto dist_sq = distanceSquared(origin, x);
			    return min_sq > dist_sq || max_sq < dist_sq;
		    });
		pc.erase(it, pc.end());
	} else if constexpr (execution::is_tbb_v<ExecutionPolicy>) {
		auto it = std::remove_if(UFO_TBB_PAR pc.begin(), pc.end(),
		                         [&origin, &min_sq, &max_sq](Vec<Dim, T> x) {
			                         auto dist_sq = distanceSquared(origin, x);
			                         return min_sq > dist_sq || max_sq < dist_sq;
		                         });
		pc.erase(it, pc.end());
	}
}
}  // namespace ufo

#endif  // UFO_PCL_POINT_CLOUD_HPP