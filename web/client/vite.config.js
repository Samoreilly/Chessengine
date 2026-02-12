import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      // Proxy WebSocket connections to the backend server during development
      '/ws': {
        target: 'ws://localhost:3000',
        ws: true,
      },
    },
  },
})
