import { mount } from 'svelte'
import './app.css'
import App from './App.svelte'

const app = mount(App, {
  target: document.getElementById('app') ?? document.body, 
  // ?? document.body resolves the error by using the body as a fallback if #app is null - even though the div now exists
})

export default app
