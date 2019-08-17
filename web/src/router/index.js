import Vue from 'vue'
import Router from 'vue-router'
import Vnc from '@/components/Vnc'

Vue.use(Router)

console.log(`within (router) /src/index.js: ${process.env.PREFIX_PATH}`)

export default new Router({
  mode: 'history',
  base: `${process.env.PREFIX_PATH}`,
  routes: [
    {
      path: '/',
      name: 'Vnc',
      component: Vnc
    }
  ]
})
