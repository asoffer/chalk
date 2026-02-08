use std::collections::HashMap;

use proc_macro2::Spacing;
use proc_macro2::TokenStream;
use proc_macro2::TokenTree;
use quote::quote;
use syn::parse_macro_input;
use syn::parse_quote;
use syn::punctuated::Punctuated;
use syn::GenericParam;
use syn::Generics;
use syn::Ident;
use syn::Item;
use syn::ItemEnum;
use syn::ItemStruct;
use syn::ItemTrait;
use syn::ItemType;
use syn::ItemUnion;
use syn::Path;
use syn::Token;
use syn::Type;
use syn::WherePredicate;

#[proc_macro_attribute]
pub fn chalk(
  attr: proc_macro::TokenStream,
  item: proc_macro::TokenStream,
) -> proc_macro::TokenStream {
  let item = parse_macro_input!(item as Item);
  let impls = chalk_impl(attr.into(), &item);

  quote! {
    #impls
    #item
  }
  .into()
}

fn chalk_impl(attr: TokenStream, item: &Item) -> TokenStream {
  let mut map = HashMap::<Path, Punctuated<WherePredicate, Token![,]>>::new();
  let mut ident_reqs =
    HashMap::<String, Punctuated<WherePredicate, Token![,]>>::new();
  for stream in split_stream(&attr).into_iter() {
    let (ident, reqs) = extract_requirements(parse_quote!(trait #stream {}));
    for marker in marker_traits(&ident).into_iter() {
      map.entry(marker).or_default().extend(reqs.clone());
    }
    ident_reqs.entry(ident).or_default().extend(reqs)
  }

  let ident = annotated_ident(item);
  let annotated_generics = annotated_generics(item);
  let generic_params = annotated_generics.params.iter().collect::<Vec<_>>();
  let parameterized_impl: TokenStream = if annotated_generics.params.is_empty()
  {
    quote!(#ident)
  } else {
    quote!(impl<#(#generic_params),*>)
  };

  let generic_idents: Vec<_> = annotated_generics
    .params
    .iter()
    .map(|g| match g {
      GenericParam::Lifetime(l) => &l.lifetime.ident,
      GenericParam::Type(t) => &t.ident,
      GenericParam::Const(c) => &c.ident,
    })
    .collect();

  let parameterized_ident: Type = if generic_idents.is_empty() {
    parse_quote!(#ident)
  } else {
    parse_quote!(#ident<#(#generic_idents),*>)
  };

  let marker_impls: Vec<_> = map
    .iter()
    .map(|(marker, predicate)| {
      quote! {
        #parameterized_impl #marker for #parameterized_ident where #predicate {}
      }
    })
    .collect();

  let generic_params = annotated_generics.params.iter().collect::<Vec<_>>();

  let impls: Vec<_> = ident_reqs
    .into_iter()
    .map(|(ident, predicate)| {
      item_impls(&ident)
        .into_iter()
        .map(|i| i.to_impls(&generic_params, &parameterized_ident, &predicate))
        .flatten()
        .collect::<Vec<_>>()
    })
    .flatten()
    .collect();

  quote! {
    #(#marker_impls)*
    #(#impls)*
  }
  .into()
}

fn marker_traits(name: &str) -> Vec<Path> {
  if name == "Ring" {
    vec![
      parse_quote!(chalk::AddAssociative),
      parse_quote!(chalk::MulAssociative),
      parse_quote!(chalk::AddMulDistributive),
    ]
  } else if name == "CommutativeRing" {
    vec![
      parse_quote!(chalk::AddAssociative),
      parse_quote!(chalk::MulAssociative),
      parse_quote!(chalk::MulCommutative),
      parse_quote!(chalk::AddMulDistributive),
    ]
  } else {
    vec![]
  }
}

enum ItemImpl {
  Add,
  Sub,
  Mul,
  #[allow(dead_code)]
  Div,
}

impl ItemImpl {
  fn to_impls(
    self,
    params: &[&GenericParam],
    ty: &Type,
    where_predicates: &Punctuated<WherePredicate, Token![,]>,
  ) -> Vec<Item> {
    match self {
      ItemImpl::Add => vec![
        parse_quote! {
          impl<#(#params),*> std::ops::Add<#ty> for #ty where #where_predicates {
            type Output = #ty;
            fn add(mut self, rhs: #ty) -> #ty {
              self += &rhs;
              self
            }
          }
        },
        parse_quote! {
          impl<'__chalk, #(#params),*> std::ops::Add<&'__chalk #ty> for #ty
              where #where_predicates {
            type Output = #ty;
            fn add(mut self, rhs: &#ty) -> #ty {
              self += rhs;
              self
            }
          }
        },
      ],
      ItemImpl::Sub => vec![
        parse_quote! {
          impl<#(#params),*> std::ops::Sub<#ty> for #ty where #where_predicates {
            type Output = #ty;
            fn sub(mut self, rhs: #ty) -> #ty {
              self -= &rhs;
              self
            }
          }
        },
        parse_quote! {
          impl<'__chalk, #(#params),*> std::ops::Sub<&'__chalk #ty> for #ty
              where #where_predicates {
            type Output = #ty;
            fn sub(mut self, rhs: &#ty) -> #ty {
              self -= rhs;
              self
            }
          }
        },
      ],
      ItemImpl::Mul => vec![
        parse_quote! {
          impl<#(#params),*> std::ops::Mul<#ty> for #ty where #where_predicates {
            type Output = #ty;
            fn mul(mut self, rhs: #ty) -> #ty {
              self *= &rhs;
              self
            }
          }
        },
        parse_quote! {
          impl<'__chalk, #(#params),*> std::ops::Mul<&'__chalk #ty> for #ty
              where #where_predicates {
            type Output = #ty;
            fn mul(mut self, rhs: &#ty) -> #ty {
              self *= rhs;
              self
            }
          }
        },
      ],
      ItemImpl::Div => vec![parse_quote! {
        impl<'__chalk, #(#params),*> std::ops::Div<&'__chalk #ty> for #ty {
          type Output = #ty;
          fn div(mut self, rhs: &#ty) -> #ty {
            self /= rhs;
            self
          }
        }
      }],
    }
  }
}

fn item_impls(name: &str) -> Vec<ItemImpl> {
  if name == "Ring" {
    vec![ItemImpl::Add, ItemImpl::Sub, ItemImpl::Mul]
  } else {
    vec![]
  }
}

fn annotated_ident(item: &Item) -> &Ident {
  match item {
    Item::Enum(ItemEnum { ident, .. }) => ident,
    Item::Struct(ItemStruct { ident, .. }) => ident,
    Item::Union(ItemUnion { ident, .. }) => ident,
    Item::Type(ItemType { ident, .. }) => ident,
    _ => panic!("Item not supported."),
  }
}

fn annotated_generics(item: &Item) -> &Generics {
  match item {
    Item::Enum(ItemEnum { generics, .. }) => generics,
    Item::Struct(ItemStruct { generics, .. }) => generics,
    Item::Union(ItemUnion { generics, .. }) => generics,
    Item::Type(ItemType { generics, .. }) => generics,
    _ => panic!("Item not supported."),
  }
}

fn extract_requirements(
  p: ItemTrait,
) -> (String, Punctuated<WherePredicate, Token![,]>) {
  let preds = p
    .generics
    .where_clause
    .map(|w| w.predicates)
    .unwrap_or(Punctuated::new());

  (p.ident.to_string(), preds)
}

fn split_stream(stream: &TokenStream) -> Vec<TokenStream> {
  let mut v = vec![TokenStream::new()];
  let mut marker: Option<TokenTree> = None;
  for token in stream.clone().into_iter() {
    match token {
      TokenTree::Punct(p) => {
        if p.as_char() == '|' && p.spacing() == Spacing::Joint {
          if let Some(prev) = marker {
            let tree = v.last_mut().unwrap();
            tree.extend([prev, TokenTree::Punct(p)]);
            marker = None;
          } else {
            marker = Some(p.into());
          }
        } else if p.as_char() == '>' && p.spacing() == Spacing::Alone {
          if marker.is_some() {
            v.push(TokenStream::new());
            marker = None;
          } else {
            v.last_mut().unwrap().extend([TokenTree::Punct(p)]);
          }
        } else {
          v.last_mut().unwrap().extend([TokenTree::Punct(p)]);
        }
      }
      t => v.last_mut().unwrap().extend([t]),
    }
  }

  if v.first().unwrap().is_empty() {
    let s = v.pop().unwrap();
    v[0] = s;
  }
  v
}
