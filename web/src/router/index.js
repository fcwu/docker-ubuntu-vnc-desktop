import Vue from 'vue'
import Router from 'vue-router'
import Vnc from '@/components/Vnc'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/',
      name: 'Vnc',
      component: Vnc
    }
  ]
})
