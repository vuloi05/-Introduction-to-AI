import { useAppStore } from '@/store/useAppStore'
import { motion } from 'framer-motion'
import { LayoutDashboard, Settings, Users, PieChart, Menu } from 'lucide-react'
import { NavLink } from 'react-router-dom'
import { cn } from '@/lib/utils'

const menuItems = [
  { icon: LayoutDashboard, label: 'Dashboard', path: '/dashboard' },
  { icon: Users, label: 'Customers', path: '/customers' },
  { icon: PieChart, label: 'Analytics', path: '/analytics' },
  { icon: Settings, label: 'Settings', path: '/settings' },
]

export default function Sidebar() {
  const { sidebarOpen, toggleSidebar } = useAppStore()

  return (
    <motion.aside
      initial={false}
      animate={{ width: sidebarOpen ? 240 : 80 }}
      className="h-screen bg-card border-r flex flex-col items-center py-6 px-4 sticky top-0"
    >
      <div className="flex w-full items-center justify-between mb-8 overflow-hidden">
        {sidebarOpen && <span className="font-bold text-xl tracking-tight text-foreground whitespace-nowrap">Acme.</span>}
        <button onClick={toggleSidebar} className="p-2 hover:bg-accent rounded-md text-foreground/70 hover:text-foreground transition-colors ml-auto">
          <Menu className="w-5 h-5" />
        </button>
      </div>

      <nav className="flex-1 w-full space-y-2">
        {menuItems.map((item) => (
          <NavLink
            key={item.path}
            to={item.path}
            className={({ isActive }) =>
              cn(
                "flex items-center py-3 px-3 rounded-lg transition-all w-full overflow-hidden whitespace-nowrap",
                isActive 
                  ? "bg-primary text-primary-foreground shadow-sm" 
                  : "text-foreground/70 hover:bg-accent hover:text-accent-foreground"
              )
            }
          >
            <item.icon className="w-5 h-5 flex-shrink-0" />
            <motion.span 
              initial={false} 
              animate={{ opacity: sidebarOpen ? 1 : 0, width: sidebarOpen ? 'auto' : 0 }}
              className="ml-3 font-medium text-sm"
            >
              {item.label}
            </motion.span>
          </NavLink>
        ))}
      </nav>
    </motion.aside>
  )
}
