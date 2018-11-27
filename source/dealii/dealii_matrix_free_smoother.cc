/*************************************************************************
 * Copyright (c) 2017-2018 by the mfmg authors                           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of the mfmg libary. mfmg is distributed under a BSD *
 * 3-clause license. For the licensing terms see the LICENSE file in the *
 * top-level directory                                                   *
 *                                                                       *
 * SPDX-License-Identifier: BSD-3-Clause                                 *
 *************************************************************************/

#include <mfmg/common/exceptions.hpp>
#include <mfmg/common/instantiation.hpp>
#include <mfmg/dealii/dealii_matrix_free_smoother.hpp>

namespace mfmg
{
template <typename VectorType>
DealIIMatrixFreeSmoother<VectorType>::DealIIMatrixFreeSmoother(
    std::shared_ptr<Operator<vector_type> const> op,
    std::shared_ptr<boost::property_tree::ptree const> params)
    : Smoother<VectorType>(op, params)
{
  std::string prec_name = params->get("smoother.type", "Chebyshev");

  auto matrix_free_operator =
      std::dynamic_pointer_cast<operator_type const>(this->_operator);
  ASSERT(matrix_free_operator != nullptr,
         "DealIIMartixFreeSmoother must be constructed from a "
         "DealIIMatrixFreeOperator");

  std::transform(prec_name.begin(), prec_name.end(), prec_name.begin(),
                 ::tolower);
  if (prec_name == "chebyshev")
  {
    _smoother.reset(new preconditioner_type());
    _smoother->initialize(*matrix_free_operator);
  }
  else
  {
    ASSERT_THROW(false, "Unknown smoother name: \"" + prec_name + "\"");
  }
}

template <typename VectorType>
void DealIIMatrixFreeSmoother<VectorType>::apply(VectorType const &b,
                                                 VectorType &x) const
{
  // r = -(b - Ax)
  vector_type r(b);
  this->_operator->apply(x, r);
  r.add(-1., b);

  // x = x + B^{-1} (-r)
  vector_type tmp(x);
  _smoother->vmult(tmp, r);
  x.add(-1., tmp);
}

} // namespace mfmg

// Explicit Instantiation
INSTANTIATE_VECTORTYPE(TUPLE(DealIIMatrixFreeSmoother))
