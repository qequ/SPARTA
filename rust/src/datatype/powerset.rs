/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::collections::HashSet;
use std::iter::FromIterator;

use super::abstract_domain::AbstractDomain;
use crate::datatype::PatriciaTreeSet;

pub trait SetAbstractDomainOps: Clone + Eq {
    fn is_subset(&self, other: &Self) -> bool;
    fn intersection_with(&mut self, other: &Self);
    fn union_with(&mut self, other: Self);
}

pub trait SetElementOps {
    type Element;
    type ElementIter<'a>: Iterator<Item = &'a Self::Element>
    where
        Self: 'a;

    fn add_element(&mut self, e: Self::Element);
    fn remove_element(&mut self, e: &Self::Element);
    fn elements(&self) -> Self::ElementIter<'_>;
}

#[derive(PartialEq, Eq, Clone, Debug)]
pub enum PowersetLattice<S: SetAbstractDomainOps> {
    Top,
    Value(S),
    Bottom,
}

impl<S: SetAbstractDomainOps> PowersetLattice<S> {
    pub fn value_from_set(set: S) -> Self {
        Self::Value(set)
    }
}

impl<S: SetAbstractDomainOps + SetElementOps> PowersetLattice<S> {
    pub fn add_element(&mut self, e: S::Element) {
        if let Self::Value(powerset) = self {
            powerset.add_element(e);
        }
    }

    pub fn add_elements<I: IntoIterator<Item = S::Element>>(&mut self, elements: I) {
        if let Self::Value(powerset) = self {
            for e in elements {
                powerset.add_element(e);
            }
        }
    }

    pub fn remove_element(&mut self, e: &S::Element) {
        if let Self::Value(powerset) = self {
            powerset.remove_element(e);
        }
    }

    pub fn remove_elements<'a, I: IntoIterator<Item = &'a S::Element>>(&mut self, elements: I)
    where
        S::Element: 'a,
    {
        if let Self::Value(powerset) = self {
            for e in elements {
                powerset.remove_element(e);
            }
        }
    }

    pub fn elements(&self) -> impl Iterator<Item = &'_ S::Element> {
        // NOTE: this is a workaround to make an empty iter, since we don't
        // know the actual type of S::ElementIter, we cannot create an empty
        // iter with the same type.
        let res = match self {
            Self::Value(powerset) => Some(powerset.elements()),
            _ => None,
        };
        res.into_iter().flatten()
    }

    pub fn set(&self) -> &S {
        match self {
            Self::Value(powerset) => powerset,
            _ => panic!("set called on Top or Bottom value!"),
        }
    }

    pub fn into_set(self) -> S {
        match self {
            Self::Value(powerset) => powerset,
            _ => panic!("into_set called on Top or Bottom value!"),
        }
    }
}

impl<S: SetAbstractDomainOps> AbstractDomain for PowersetLattice<S> {
    fn bottom() -> Self {
        Self::Bottom
    }

    fn top() -> Self {
        Self::Top
    }

    fn is_bottom(&self) -> bool {
        matches!(self, Self::Bottom)
    }

    fn is_top(&self) -> bool {
        matches!(self, Self::Top)
    }

    fn leq(&self, rhs: &Self) -> bool {
        match self {
            Self::Top => rhs.is_top(),
            Self::Value(ref s) => match rhs {
                Self::Top => true,
                Self::Value(ref t) => s.is_subset(t),
                Self::Bottom => false,
            },
            Self::Bottom => true,
        }
    }

    fn join_with(&mut self, rhs: Self) {
        match self {
            Self::Top => {}
            Self::Value(ref mut s) => match rhs {
                Self::Top => *self = rhs,
                Self::Value(t) => {
                    s.union_with(t);
                }
                Self::Bottom => {}
            },
            Self::Bottom => *self = rhs,
        };
    }

    fn meet_with(&mut self, rhs: Self) {
        match self {
            Self::Top => *self = rhs,
            Self::Value(ref mut s) => match rhs {
                Self::Top => {}
                Self::Value(ref t) => {
                    s.intersection_with(t);
                }
                Self::Bottom => *self = rhs,
            },
            Self::Bottom => {}
        };
    }

    fn widen_with(&mut self, _rhs: Self) {
        *self = Self::top();
    }

    fn narrow_with(&mut self, _rhs: Self) {
        *self = Self::bottom();
    }
}

impl<S, A> FromIterator<A> for PowersetLattice<S>
where
    S: SetAbstractDomainOps + FromIterator<A>,
{
    fn from_iter<T: IntoIterator<Item = A>>(iter: T) -> Self {
        Self::value_from_set(S::from_iter(iter))
    }
}

pub type PatriciaTreeSetAbstractDomain<T> = PowersetLattice<PatriciaTreeSet<T>>;
pub type HashSetAbstractDomain<T> = PowersetLattice<HashSet<T>>;
