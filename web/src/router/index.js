import Vue from 'vue'
import Router from 'vue-router'
import Vnc from '@/components/Vnc'

Vue.use(Router)

export default new Router({
  mode: 'history',
  base: window.location.pathname,
  routes: [
    {
      path: '/',
      name: 'Vnc',
      component: Vnc
    }
  ]
})
