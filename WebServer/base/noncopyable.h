/*
 * File: noncopyable.h
 * Project: base
 * Author: Sandy
 * Last Modified: 2020-10-30 19:56:43
 */

#pragma once

class noncopyable {
  protected:
    noncopyable() {}
    ~noncopyable() {}

  private:
    noncopyable(const noncopyable &);
    const noncopyable &operator=(const noncopyable &);
};
