/*

Copyright (c) 2005-2016, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Aboria.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef FAST_MULTIPOLE_METHOD_DETAIL_H_
#define FAST_MULTIPOLE_METHOD_DETAIL_H_

#include "detail/SpatialUtil.h"
#include "detail/Chebyshev.h"
#include "NeighbourSearchBase.h"
#include "Traits.h"
#include "CudaInclude.h"
#include "Vector.h"
#include "Get.h"
#include "Log.h"
#include <iostream>

namespace Aboria {
namespace detail {

    template <unsigned int D, unsigned int N> 
    struct MultiquadricExpansions {
        typedef detail::bbox<D> box_type;
        static const size_t ncheb = std::pow(N,D); 
        typedef std::array<double,ncheb> expansion_type;
        typedef Vector<double,D> double_d;
        typedef Vector<int,D> int_d;
        static const unsigned int dimension = D;
        const double m_c2;

        MultiquadricExpansions(const double c):m_c2(c*c) 
        {}

        static void P2M(expansion_type& accum, 
                 const box_type& box, 
                 const double_d& position,
                 const double& source ) {

        }

        static void M2M(expansion_type& accum, 
                 const box_type& target_box, 
                 const box_type& source_box, 
                 const expansion_type& source) {

        }

        void M2L(expansion_type& accum, 
                 const box_type& target_box, 
                 const box_type& source_box, 
                 const expansion_type& source) {

          
        }

        static void L2L(expansion_type& accum, 
                 const box_type& target_box, 
                 const box_type& source_box, 
                 const expansion_type& source) {
         
        }

        static double L2P(const double_d& p,
                   const box_type& box, 
                   const expansion_type& source) {
            return 0.0;
        }

    };


    template <unsigned int D, unsigned int N, typename Function> 
    struct BlackBoxExpansions {
        
        typedef detail::bbox<D> box_type;
        static constexpr size_t ncheb = ipow(N,D); 
        typedef std::array<double,ncheb> expansion_type;
        typedef Vector<double,D> double_d;
        typedef Vector<int,D> int_d;
        static const unsigned int dimension = D;
        static const unsigned int number_of_nodes_in_each_direction = N;
        Function m_K;

        BlackBoxExpansions(const Function &K):m_K(K) 
        {}

        static void P2M(expansion_type& accum, 
                 const box_type& box, 
                 const double_d& position,
                 const double& source ) {

            detail::ChebyshevRnSingle<D,N> cheb_rn(position,box);
            lattice_iterator<dimension> mj(int_d(0),int_d(N));
            for (int j=0; j<ncheb; ++j,++mj) {
                //std::cout << "accumulating P2M from "<<position<<" to node "<<*mj<<" with Rn = "<<cheb_rn(*mj)<<std::endl;
                accum[j] += cheb_rn(*mj)*source;
            }
        }

        static void M2M(expansion_type& accum, 
                 const box_type& target_box, 
                 const box_type& source_box, 
                 const expansion_type& source) {

            lattice_iterator<dimension> mj(int_d(0),int_d(N));
            for (int j=0; j<ncheb; ++j,++mj) {
                const double_d pj_unit_box = detail::chebyshev_node_nd(*mj,N);
                const double_d pj = 0.5*(pj_unit_box+1)*(source_box.bmax-source_box.bmin) 
                    + source_box.bmin;
                detail::ChebyshevRnSingle<D,N> cheb_rn(pj,target_box);

                lattice_iterator<D> mi(int_d(0),int_d(N));
                for (int i=0; i<ncheb; ++i,++mi) {
                    accum[i] += cheb_rn(*mi)*source[j];
                }
            }
            /*
            lattice_iterator<dimension> mi(int_d(0),int_d(N-1),int_d(0));
            for (int i=0; i<ncheb; ++i,++mi) {
                const double_d pi_unit_box = detail::chebyshev_node_nd(*mi,N);
                const double_d pi = 0.5*(pi_unit_box+1)*(target_box.bmax-target_box.bmin) 
                    + target_box.bmin;
                detail::ChebyshevRnSingle<D,N> cheb_rn(pi,source_box);

                lattice_iterator<D> mj(int_d(0),int_d(N-1),int_d(0));
                for (int j=0; j<ncheb; ++j,++mj) {
                    accum[i] += cheb_rn(*mj)*source[j];
                }
            }
            */
        }


        void M2L(expansion_type& accum, 
                 const box_type& target_box, 
                 const box_type& source_box, 
                 const expansion_type& source) {

            lattice_iterator<dimension> mi(int_d(0),int_d(N));
            for (int i=0; i<ncheb; ++i,++mi) {
                const double_d pi_unit_box = detail::chebyshev_node_nd(*mi,N);
                const double_d pi = 0.5*(pi_unit_box+1)*(target_box.bmax-target_box.bmin) 
                                                                    + target_box.bmin;

                lattice_iterator<dimension> mj(int_d(0),int_d(N));
                for (int j=0; j<ncheb; ++j,++mj) {
                    const double_d pj_unit_box = detail::chebyshev_node_nd(*mj,N);
                    const double_d pj = 0.5*(pj_unit_box+1)*(source_box.bmax-source_box.bmin) 
                                                                    + source_box.bmin;
                    accum[i] += m_K(pj-pi,pi,pj)*source[j];
                }
            }


        }

        static void L2L(expansion_type& accum, 
                 const box_type& target_box, 
                 const box_type& source_box, 
                 const expansion_type& source) {
            //M2M(accum,target_box,source_box,source);
            lattice_iterator<dimension> mi(int_d(0),int_d(N));
            for (int i=0; i<ncheb; ++i,++mi) {
                const double_d pi_unit_box = detail::chebyshev_node_nd(*mi,N);
                const double_d pi = 0.5*(pi_unit_box+1)*(target_box.bmax-target_box.bmin) 
                    + target_box.bmin;
                detail::ChebyshevRnSingle<D,N> cheb_rn(pi,source_box);

                lattice_iterator<D> mj(int_d(0),int_d(N));
                for (int j=0; j<ncheb; ++j,++mj) {
                    accum[i] += cheb_rn(*mj)*source[j];
                }
            }
            /*
            lattice_iterator<dimension> mj(int_d(0),int_d(N-1),int_d(0));
            for (int j=0; j<ncheb; ++j,++mj) {
                const double_d pj_unit_box = detail::chebyshev_node_nd(*mj,N);
                const double_d pj = 0.5*(pj_unit_box+1)*(source_box.bmax-source_box.bmin) 
                    + source_box.bmin;
                detail::ChebyshevRnSingle<D,N> cheb_rn(pj,target_box);

                lattice_iterator<D> mi(int_d(0),int_d(N-1),int_d(0));
                for (int i=0; i<ncheb; ++i,++mi) {
                    accum[i] += cheb_rn(*mi)*source[j];
                }
            }
            */
        }

        static double L2P(const double_d& p,
                   const box_type& box, 
                   const expansion_type& source) {
            detail::ChebyshevRnSingle<D,N> cheb_rn(p,box);
            lattice_iterator<dimension> mj(int_d(0),int_d(N));
            double sum = 0;
            for (int j=0; j<ncheb; ++j,++mj) {
                //std::cout << "cheb node "<<*mj<<" p = "<<cheb_rn.get_position(*mj)<<" source = "<<source[j]<<std::endl;
                sum += cheb_rn(*mj)*source[j];
            }
            return sum;
        }

    };



    template <typename Expansions,
              typename Traits, 
              typename SourceVectorType, 
                    typename VectorType=typename Expansions::expansion_type,
                    typename SourceParticleIterator=typename Traits::raw_pointer, 
                    unsigned int D=Traits::dimension>
    void calculate_P2M(VectorType& sum, 
                        const detail::bbox<D>& box, 
                        const iterator_range<ranges_iterator<Traits>>& range, 
                        const SourceVectorType& source_vector,
                        const SourceParticleIterator& source_particles_begin,
                        const Expansions& expansions) {
        typedef typename Traits::position position;
        const size_t N = std::distance(range.begin(),range.end());
        const Vector<double,D>* pbegin = &get<position>(*range.begin());
        const size_t index = pbegin - &get<position>(source_particles_begin)[0];
        for (int i = 0; i < N; ++i) {
            const Vector<double,D>& pi = pbegin[i]; 
            expansions.P2M(sum,box,pi,source_vector[index+i]);  
        }
    }

    // assume serial processing of particles, this could be more efficient for ranges iterators
    template <typename Expansions,
                typename Iterator, 
              typename SourceVectorType, 
                 typename VectorType=typename Expansions::expansion_type, 
                 typename Traits=typename Iterator::traits_type,
                 typename SourceParticleIterator=typename Traits::raw_pointer, 
                 unsigned int D=Traits::dimension,
                 typename = typename
        std::enable_if<!std::is_same<Iterator,ranges_iterator<Traits>>::value>>
    void calculate_P2M(VectorType& sum, 
                        const detail::bbox<D>& box, 
                        const iterator_range<Iterator>& range, 
                        const SourceVectorType& source_vector,
                        const SourceParticleIterator& source_particles_begin,
                        const Expansions &expansions) {

        typedef typename Traits::position position;
        typedef typename Iterator::reference reference;
        for (reference i: range) {
            const Vector<double,D>& pi = get<position>(i); 
            const size_t index = &pi- &get<position>(source_particles_begin)[0];
            expansions.P2M(sum,box,pi,source_vector[index]);  
        }

    }


    template <typename Traits, 
              typename Expansions, 
              typename SourceVectorType,
                    typename SourceParticleIterator=typename Traits::raw_pointer, 
                    unsigned int D=Traits::dimension>
    double calculate_K_direct(const Vector<double,D>& p,
                            const iterator_range<ranges_iterator<Traits>>& range, 
                            Expansions& expansions,
                            const SourceVectorType& source_vector,
                            const SourceParticleIterator& source_particles_begin) {
        typedef typename Traits::position position;
        typedef Vector<double,D> double_d;
        const size_t N = std::distance(range.begin(),range.end());
        const double_d* pbegin = &get<position>(*range.begin());
        const size_t index = pbegin - &get<position>(source_particles_begin)[0];
        double sum = 0;
        for (int i = 0; i < N; ++i) {
            const Vector<double,D>& pi = pbegin[i]; 
            sum += expansions.m_K(pi-p,p,pi)*source_vector[index+i];
        }
        return sum;
    }

    template <typename Iterator, 
              typename Expansions, 
              typename SourceVectorType,
                    typename Traits=typename Iterator::traits_type,
                    typename SourceParticleIterator=typename Traits::raw_pointer, 
                    unsigned int D=Traits::dimension,
                    typename = typename
        std::enable_if<!std::is_same<Iterator,ranges_iterator<Traits>>::value>>
    double calculate_K_direct(const Vector<double,D>& p,
                            const iterator_range<Iterator>& range, 
                            Expansions& expansions,
                            const SourceVectorType& source_vector,
                            const SourceParticleIterator& source_particles_begin) {
        typedef typename Traits::position position;
        typedef typename Iterator::reference reference;
        
        double sum = 0;
        for (reference i: range) {
            const Vector<double,D>& pi = get<position>(i); 
            const size_t index = &pi- &get<position>(source_particles_begin)[0];
            sum += expansions.m_K(pi-p,p,pi)*source_vector[index];
        }
        return sum;
    }

    template <unsigned int D>
    struct theta_condition {
        typedef Vector<double,D> double_d;
        const double_d& m_low;
        const double_d& m_high;
        const double m_r2;
        const double m_r;
        static constexpr double m_theta = 0.5;
        static constexpr double m_theta2 = m_theta*m_theta;
        theta_condition(const double_d& low, const double_d& high):
            m_low(low),m_high(high),
            m_r2(0.25*(high-low).squaredNorm()),
            m_r(std::sqrt(m_r2))
        {}

        bool check(const double_d& low, const double_d& high) const {
            double d = 0.5*(high + low - m_low - m_high).norm(); 
            double other_r2 = 0.25*(high-low).squaredNorm();
            if (other_r2 < m_r2) {
                const double other_r = std::sqrt(other_r2);
                return m_r2 > m_theta2*std::pow(d-other_r,2);
            } else {
                return other_r2 > m_theta2*std::pow(d-m_r,2);
            }
        }
    };

}
}

#endif