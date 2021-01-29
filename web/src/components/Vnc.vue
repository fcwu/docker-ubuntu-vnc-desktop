<template>
  <div style="width: 100%; height: 100%; background-color: #000;">
    <iframe id="videoFrame" ref="videoFrame" class="frame" frameBorder="0" v-show="config.mode === 'video'" scrolling="no"></iframe>
    <iframe id="vncFrame" ref="vncFrame" class="frame" v-bind:class="{hiddenvnc: config.mode === 'video'}" frameBorder="0" v-show="true" scrolling="no"></iframe>
  </div>
</template>

<script>

export default {
  name: 'Vnc',
  components: {
  },
  data () {
    return {
      // stopped -> connected -> disconnected
      vncState: 'stopped',
      videoState: 'stopped',
      // toolbar
      config: {
        mode: 'vnc'
      },
      stateID: -1,
      // retry
      errorMessage: '',
      // vnc canvas size
      width: 0,
      height: 0,
      //
      videoCurrentTime: 0,
      stateErrorCount: 0,
      timerState: null
    }
  },
  created: function () {
    window.addEventListener('message', this.onMessage)
    if ('video' in this.$route.query) {
      this.config.mode = 'video'
    }
  },
  mounted: function () {
    this.update_status()
  },
  beforeDestroy: function () {
    clearTimeout(this.timerState)
    window.removeEventListener('message', this.onMessage)
  },
  methods: {
    update_status: async function () {
      const w = this.$refs.vncFrame.clientWidth
      const h = this.$refs.vncFrame.clientHeight
      const params = {
        'video': this.config.mode === 'video',
        'id': this.stateID,
        'w': w,
        'h': h
      }
      try {
        const response = await this.$http.get('api/state', {params: params})
        const body = response.data
        if (body.code !== 200) {
          this.stateErrorCount += 1
          if (this.stateErrorCount > 10) {
            this.errorMessage = this.$t('serviceIsUnavailable')
            throw this.errorMessage
          }
        }

        // long polling
        this.stateID = body.data.id

        // adaptive resolution
        if (!body.data.config.fixedResolution && body.data.config.sizeChangedCount === 0) {
          const response = await this.$http.get('api/reset', {params: params})
          const body = response.data
          if (body.code !== 200) {
            this.stateErrorCount += 1
            if (this.stateErrorCount > 10) {
              this.errorMessage = this.$t('serviceIsUnavailable')
              throw this.errorMessage
            }
          }
        }

        if (this.vncState === 'stopped') {
          this.reconnect(false)
        }

        // video
        // try {
        //   let flvPlayer = this.$refs.videoFrame.contentWindow.flvPlayer
        //   let readyState = 0
        //   readyState = flvPlayer._mediaElement.readyState
        //   if (readyState >= 3) {
        //     this.videoState = 'running'
        //     if (this.videoCurrentTime !== flvPlayer.currentTime * 1000) {
        //       // playing
        //       let diff = (flvPlayer._mediaElement.buffered.end(0) - flvPlayer.currentTime) * 1000
        //       // console.log('player diff=' + diff)
        //       if (diff >= 2000) {
        //         // seek to nearest
        //         console.log('seek to nearest')
        //         flvPlayer._mediaElement.currentTime = flvPlayer._mediaElement.buffered.end(0)
        //       }
        //       this.videoCurrentTime = flvPlayer.currentTime * 1000
        //     } else {
        //       // stall
        //       console.log('stall, restart')
        //       this.videoState = 'stopped'
        //     }
        //   }
        // } catch (e) {
        //   // mediaElement TypeError
        // }

        this.schedule_next_update_status()
      } catch (error) {
        this.stateErrorCount += 1
        if (this.stateErrorCount > 10) {
          this.errorMessage = this.$t('serviceIsUnavailable')
        } else {
          this.schedule_next_update_status()
        }
      }
    },
    schedule_next_update_status: function (afterMseconds = 1000) {
      if (this.timerState !== null) {
        return
      }
      this.timerState = setTimeout(() => {
        this.timerState = null
        this.update_status()
      }, afterMseconds)
    },
    reconnect: function (force = false) {
      // console.trace()
      console.log(`connecting...`)
      this.errorMessage = ''
      let websockifyPath = location.pathname.substr(1) + 'websockify'
      if (force || this.vncState === 'stopped') {
        this.vncState = 'connecting'
        let hostname = window.location.hostname
        let port = window.location.port
        if (!port) {
          port = window.location.protocol[4] === 's' ? 443 : 80
        }
        let url = 'static/vnc.html?'
        url += 'autoconnect=1&'
        url += `host=${hostname}&port=${port}&`
        url += `path=${websockifyPath}&title=novnc2&`
        url += `logging=warn`
        this.$refs.vncFrame.setAttribute('src', url)
      }
      if (this.config.mode === 'video') {
        if (force || this.videoState === 'stopped') {
          const w = this.$refs.vncFrame.clientWidth
          const h = this.$refs.vncFrame.clientHeight
          let url = `static/video.html?width=${w}&height=${h}&base=${window.location.host}`
          this.$refs.videoFrame.setAttribute('src', url)
          this.videoState = 'connecting'
        }
      } else {
        if (this.videoState !== 'stopped') {
          this.$refs.videoFrame.setAttribute('src', '')
          this.videoState = 'stopped'
        }
      }
    },
    onMessage: function (message) {
      try {
        let data = JSON.parse(message.data)
        if (data.from === 'flvjs') {
          if (data.type === 'event') {
            console.log(data.eventName)
            if (data.eventName === 'onSourceOpen') {
              this.videoState = 'running'
            } else if (data.eventName === 'onSourceEnded') {
            } else if (data.eventName === 'onSourceClose') {
              this.videoState = 'stopped'
            }
          }
        }
        if (data.from === 'novnc') {
          if (data.state) {
            this.vncState = data.state
          }
        }
      } catch (exc) {
        // SyntaxError if JSON pasrse error
      }
    }
  },
  computed: {
  },
  watch: {
  }
}
</script>

<style scoped>
body {
    margin: 0px;
}

iframe {
    border-width: 0px;
    width: 100%;
    height: 100%;
    position: absolute;
    left: 0px;
    top: 0px;
}

.hiddenvnc {
  opacity: 0;
}
</style>
